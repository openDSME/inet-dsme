---
layout: post
date: 2016-08-26 12:30:44 
extra_js: 
    - http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.1/jquery.js
    - https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.1.6/Chart.min.js
    - https://cdnjs.cloudflare.com/ajax/libs/jquery-csv/0.71/jquery.csv-0.71.min.js
resultdir: /inet-dsme/results/2016-08-26-Add-code-to-display-GTS-allocation-over-time-on-website-093e4c7
title: Add code to display GTS allocation over time on website.
---
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
<video width="400" controls>
<source src="{{page.resultdir}}/gts_allocation.mp4" type="video/mp4" />
</video>
</div>
<div style="text-align:center">
<img width="400" src="{{page.resultdir}}/gts_allocation.png" width="400" alt="GTS allocation over time">
</div>