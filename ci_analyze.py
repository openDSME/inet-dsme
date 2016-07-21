#!/usr/bin/env python

import textwrap
import subprocess
import datetime
import argparse

def printcmd(args,shell):
    print "$",
    if shell:
        print args
    else:
        print " ".join(args)

def call(args,shell=False):
    printcmd(args,shell)
    return subprocess.call(args,shell=shell)

def check_output(args,shell=False):
    printcmd(args,shell)
    return subprocess.check_output(args,shell=shell)

def main(args):
    date = datetime.datetime.fromtimestamp(int(check_output(['git','log','-1','--pretty=%ct']).strip())).strftime("%Y-%m-%d")
    sanitized_commit = check_output(['git','log','-1','--pretty=%f-%h']).strip()
    subject = check_output(['git','log','-1','--pretty=%s']).strip()
    resultdir = '/inet-dsme/results/'+date+'-'+sanitized_commit

    call(['utils/analyze.py','results'])

    index = textwrap.dedent("""\
        ---
        layout: post
        extra_js: 
            - http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.1/jquery.js
            - https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.1.6/Chart.min.js
            - https://cdnjs.cloudflare.com/ajax/libs/jquery-csv/0.71/jquery.csv-0.71.min.js
        resultdir: %s
        title: %s
        ---\n"""%(resultdir,subject))
    index += textwrap.dedent("""\
        <canvas id="chartDSME" width="800" height="200"></canvas>
        <canvas id="chartCSMA" width="800" height="200"></canvas>
        <script>
        $(document).ready(function () {
            $.ajax({
                type: "GET",
                url: "{{page.resultdir}}/per_host.csv",
                dataType: "text",
                success: function (data) { processData(data); }
                });

            function processData(content) {
                data = $.csv.toObjects(content);

                colors = {};
                colors['DSME'] = "rgba(18,74,83,1)";
                colors['CSMA'] = "rgba(104,125,45,1)";

                ['DSME','CSMA'].forEach(function(type) {
                    labels = [];
                    prrs = [];
                    for (var i = 0, len = data.length; i < len; i++) {
                        labels.push(data[i]['address']);
                        prr = parseFloat(data[i][type+'-2.25-0.0-received'])/(parseFloat(data[i][type+'-2.25-0.0-lost'])+parseFloat(data[i][type+'-2.25-0.0-received']));
                        prrs.push(prr);
                    }

                    var ctx = document.getElementById("chart"+type);
                    var myChart = new Chart(ctx, {
                        type: 'bar',
                        data: {
                            labels: labels,
                            datasets: [{
                                label: 'PRR '+type,
                                data: prrs,
                                backgroundColor: colors[type]
                            }]
                        },
                        options: {
                            scales: {
                                yAxes: [{
                                    ticks: {
                                       suggestedMin: 0.8 
                                    },
                                }]
                            }
                        }
                    });
                });
            }

        });
        </script>

        <div style="text-align:center">
        <video width="400" controls>
        <source src="{{page.resultdir}}/gts_allocation.mp4" type="video/mp4" />
        </video>
        </div>""")

    with open('results/index.md','w') as indexfile:
        indexfile.write(index)

    if args.video:
        call(['utils/gts_allocation.py','--no-show','-l','results/DSME-mac.log','-o','results/gts_allocation.mp4'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-v','--video', action='store_true', help="Generate videos")
    args = parser.parse_args()
    main(args)