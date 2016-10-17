import 'package:angular2/core.dart';

import 'dart:html';
import 'dart:async';
import 'dart:convert' show JSON;

import 'package:wamp/wamp_client.dart';

String serverUri = 'ws://localhost:9002';

class InputClient extends WampClient {
    InputClient(WebSocket socket) : super(socket) {
        this.socket = socket;
    }

    void send(double value) {
        var callId = generateSessionId();
        var uri = "http://example.com/simple/setInterval";
        var arg = value;
        const int CALL = 2;
        var msg = [CALL, callId, uri, arg];
        socket.send(JSON.encode(msg));
    }

    WebSocket socket;
}

@Component(
        selector: 'slider-input',
        template: '''
            <input #input (change)="onChange()" (input)="onInput()" type="range" min="1" max="40">
            <span #value >0.0</span>
        ''')

class SliderInput {
    @ViewChild('input') ElementRef inputElement;
    @ViewChild('value') ElementRef valueElement;

    SliderInput() {
        WebSocket socket     = new WebSocket(serverUri);
        this.client          = new InputClient(socket);
    }

    onChange() {
        double value = double.parse(inputElement.nativeElement.value) / 10;
        valueElement.nativeElement.text = value.toString();
        client.send(value);
    }

    onInput() {
        double value = double.parse(inputElement.nativeElement.value) / 10;
        valueElement.nativeElement.text = value.toString();
    }

    InputClient client;
}
