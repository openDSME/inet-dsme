% ===============================================================================
% optiPIDctrl                      Lukas Reichlin                   February 2012
% ===============================================================================
% Return PID controller with roll-off for given parameters Kp, Ti and Td.
% ===============================================================================

function C = optiPIDctrl (Kp, Ti, Td)

  global dt

  %tau = Td / 10;    % roll-off

  %num = Kp * [Ti*Td, Ti, 1];
  %den = conv ([Ti, 0], [tau^2, 2*tau, 1]);
  
  %C = tf (num, den);
  
  Ki = Kp/Ti;
  Kd = Kp*Td;
  
  C = pid(Kp,Ki,Kd,2);
  
  C = c2d(C,dt);
  
  %z = tf('z',dt);
  %ifz = (dt/2)*(z+1)/(z-1);
  %dfz = dt*z/(z-1);
  %N = 2.3;
  %C = Kp * (1+((1/Ti)*ifz)+(Td/((Td/N)+dfz)));
  %C = Kp * (1+((1/Ti)*ifz));

end

% ===============================================================================
