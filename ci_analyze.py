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

def check_output(args,shell=False,cwd=None):
    printcmd(args,shell)
    return subprocess.check_output(args,shell=shell,cwd=cwd)

def main(args):
    vals = {}
    dsme_path = 'src/openDSME'
    for d in ['.',dsme_path]:
        vals[d] = {}
        vals[d]['datetime'] = datetime.datetime.fromtimestamp(int(check_output(['git','log','-1','--pretty=%ct'],cwd=d).strip()))
        vals[d]['date'] = vals[d]['datetime'].strftime("%Y-%m-%d")
        vals[d]['datetime_str'] = vals[d]['datetime'].strftime("%F %T %z")
        vals[d]['sanitized_commit'] = check_output(['git','log','-1','--pretty=%f-%h'],cwd=d).strip()
        vals[d]['subject'] = check_output(['git','log','-1','--pretty=%s'],cwd=d).strip()

    if vals['.']['datetime'] > vals[dsme_path]['datetime']:
        vals = vals['.']
    else:
        vals = vals[dsme_path]

    resultdir = '/inet-dsme/results/'+vals['date']+'-'+vals['sanitized_commit']

    call(['utils/analyze.py','results'])

    index = textwrap.dedent("""\
        ---
        layout: post
        date: %s
        extra_js: 
            - http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.1/jquery.js
            - https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.1.6/Chart.min.js
            - https://cdnjs.cloudflare.com/ajax/libs/jquery-csv/0.71/jquery.csv-0.71.min.js
        resultdir: %s
        title: %s
        ---\n"""%(vals['datetime_str'],resultdir,vals['subject']))
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
                    key = 0;
                    for(var k in data[0]) {
                        expr = new RegExp(type+'-.*-.*-');
                        res = expr.exec(k);
                        if(res) {
                            key = res;
                        }
                    }

                    for (var i = 0, len = data.length; i < len; i++) {
                        labels.push(data[i]['address']);
                        prr = parseFloat(data[i][key+'received'])/(parseFloat(data[i][key+'lost'])+parseFloat(data[i][key+'received']));
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
        <video style="width:100%; max-width:400px" controls>
        <source src="{{page.resultdir}}/gts_allocation.mp4" type="video/mp4" />
        </video>
        </div>
        <div style="text-align:center">
        <h2>GTS Allocation and Deallocation Over Time</h2>
        <img style="width:100%; max-width:800px" src="{{page.resultdir}}/gts_allocation.png" width="400" alt="GTS allocation over time">
        </div>""")

    with open('results/index.md','w') as indexfile:
        indexfile.write(index)

    if args.video:
        call(['utils/gts_allocation.py','--no-show','-l','results/DSME-mac.log','-o','results/gts_allocation.mp4'])
    call(['utils/gts_allocation_time.py','-l','results/DSME-mac.log','-o','results/gts_allocation.png', '-i'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-v','--video', action='store_true', help="Generate videos")
    args = parser.parse_args()
    main(args)
