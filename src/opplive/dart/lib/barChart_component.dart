import 'package:angular2/core.dart';

import 'dart:html';
import 'dart:js';

import 'package:chartjs/chartjs.dart';
import 'package:wamp/wamp_client.dart';

//String serverUri = 'ws://134.28.77.241:9002';
String serverUri = 'ws://localhost:9002';

class ChartClient extends WampClient {
    ChartClient(WebSocket socket, LinearInstance chart) : super(socket) {
        this.chart = chart;
        this.latestLabel = 0;
    }

    onWelcome() {
        subscribe('http://opendsme.org/events/1');
    }

    onEvent(topicUri, event) {
        var p = event.split(",");
        double val1 = double.parse(p[0]);
        double val2 = double.parse(p[1]);

        this.chart.addData([val1,val2], (++latestLabel).toString());
        if(latestLabel > 40) {
            this.chart.removeData();
        }
    }

    LinearInstance chart;
    int latestLabel;
}

@Component(
        selector: 'bar-chart',
        template: '<canvas #name width="500" height="400">')

class BarChart implements AfterViewInit {
    @ViewChild('name') ElementRef element;
    
    BarChart() {
    }

    void ngAfterViewInit() {
        var data = new LinearChartData(
                labels: [],
                datasets: <ChartDataSet>[
                new ChartDataSet(
                    fillColor: "rgba(0,220,0,0.2)",
                    strokeColor: "rgba(0,220,0,1)",
                    pointColor: "rgba(0,220,0,1)",
                    data: []),
                new ChartDataSet(
                    fillColor: "rgba(220,0,0,0.2)",
                    strokeColor: "rgba(220,0,0,1)",
                    pointColor: "rgba(220,0,0,1)",
                    data: [])
                ]);

        LinearInstance chart = new Chart(element.nativeElement.context2D).Bar(data);

        WebSocket socket     = new WebSocket(serverUri);
        ChartClient client   = new ChartClient(socket, chart);
    }
}
