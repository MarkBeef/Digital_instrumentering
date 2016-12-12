void compass(signed int16 Bx,signed int16 By,signed int16 Bz, signed int16 Gx, signed int16 Gy, signed int16 Gz) {
   
   
   float phi, theta, psi;
  
   
   // Calculating the roll angle Phi radians:
   if(Gz == 0 && Gy < 0) {
   
      phi = Pi/2;   // maybe check this later...
      
   } else if(Gz == 0 && Gy > 0) {
      
      phi = -PI/2;  // maybe.. check this later...
      
   } else {
   
     phi = atan((float)Gy/(float)Gz);
   }
   
   
   // calculating pitch angle Theta in radians:
   
   if( Gx > 0) {
      
      theta = Pi/2;
      
   } else if(Gx < 0) {
      
      theta = -Pi/2;
      
   } else {
   
      float denominator = (float)Gy*sin(phi) + (float)Gz*cos(phi);
      theta = atan( (float)Gx*(-1)/( denominator ));
   
   }
   
   
   // Calculating yaw:
   
   if(Bx == 0) {
      psi = -Pi/2;
   }
   
   psi = atan( (-1)*(float)By / (float)Bx );
   
}
