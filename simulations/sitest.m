pkg load control

InputData = dlmread('foo.csv',',',1,0);
input = InputData(:,1);
%input_short = input(10 : 10 : end);
output = InputData(:,2);
%output_short = output(10 : 10 : end);
%input = rand(10,1);
sampletime = 1;
t = 0 : sampletime : sampletime*(rows(input)-1);

%[input,t] = gensig('square',4,10,0.1);

%s = tf('s');
%z = tf('z',sampletime)
%[2 5 1],[1 2 3])
%G = (2*z+4)/(z*z+2*z+1)
%s = tf([2],[1])
%H = s
%G = c2d(H,sampletime)
%G = 1/z
%[output, t, x] = lsim(G,input,t);

%plot(t,input,'r');
%hold on;
%plot(t,output,'b');


dat = iddata(output, input, sampletime);

%[sys, x0] = arx(dat,  'na',2, 'nb', 2, 'nk', 0);
[sys, x0] = arx(dat,300);
tf(sys);
[y, t, x] = lsim (sys,input,t,x0);
figure
plot(t,input,'b');
hold on;
plot(t,y,'r');
hold on;
plot(t,output,'g');
hold on;
