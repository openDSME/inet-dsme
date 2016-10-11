import 'dart:html';
import 'dart:js';

import 'package:polymer/polymer.dart';

import 'package:wamp/wamp_client.dart';
import 'package:chartjs/chartjs.dart';

String serverUri = 'ws://134.28.77.241:9002';

class ChartClient extends WampClient {
    ChartClient(WebSocket socket, LinearInstance chart) : super(socket) {
        this.chart = chart;
        this.latestLabel = 0;
    }

    onWelcome() {
        subscribe('http://example.com/simple/ev4');
    }

    onEvent(topicUri, event) {
        var p = event.split(",");
        double val1 = double.parse(p[0]);
        double val2 = double.parse(p[1]);

        this.chart.addData([val1,val2], (++latestLabel).toString());
    }

    var chart;
    int latestLabel;
}

void main() {
    CanvasElement chartContainer = new CanvasElement();
    chartContainer.style.height ='400px';
    chartContainer.style.width =  '100%';
    chartContainer.id = 'canvas';
    document.body.children.add(chartContainer);

    var ctx = (querySelector('#canvas') as CanvasElement).context2D;
    var data = new LinearChartData(
            labels: [],
            datasets: <ChartDataSet>[
            new ChartDataSet(
                fillColor: "rgba(0,220,0,0.2)",
                strokeColor: "rgba(0,220,0,1)",
                pointColor: "rgba(0,220,0,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(220,220,220,1)",
                data: []),
            new ChartDataSet(
                fillColor: "rgba(220,0,0,0.2)",
                strokeColor: "rgba(220,0,0,1)",
                pointColor: "rgba(220,0,0,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(151,187,205,1)",
                data: [])
            ]);

    LinearInstance chart = new Chart(ctx).Bar(data);

    WebSocket socket = new WebSocket(serverUri);
    ChartClient client = new ChartClient(socket, chart);
}
