---
layout: post
date: 2016-08-18 12:25:06 +0000
extra_js: 
    - http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.1/jquery.js
    - https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.1.6/Chart.min.js
    - https://cdnjs.cloudflare.com/ajax/libs/jquery-csv/0.71/jquery.csv-0.71.min.js
resultdir: /inet-dsme/results/2016-08-18-Use-normal-communication-range-again-f01b379
title: Use normal communication range again.
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
</div>
