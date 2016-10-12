import 'dart:html';
import 'dart:js';

import 'package:angular2/core.dart';

import 'package:angular2/platform/browser.dart';
import 'package:webplot/sliderInput_component.dart';
import 'package:webplot/barChart_component.dart';

void main() {
    bootstrap(BarChart);
    bootstrap(SliderInput);
}
