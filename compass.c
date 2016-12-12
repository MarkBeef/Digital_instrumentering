void compass(signed int16 Bx,signed int16 By,signed int16 Bz, signed int16 Gx, signed int16 Gy, signed int16 Gz) {
   
   
   float phi, theta, psi;
   int16 bxp = Bx;
   int16 byp = By;
   int16 bzp = Bz;
   int16 gxp = Gx;
   int16 gyp = Gy;
   int16 gzp = Gz;
  
   
   // Calculating the roll angle Phi radians:
   if(Gz == 0 && Gy < 0) {
   
      phi = Pi/2;   // maybe check this later...
      
   } else if(Gz == 0 && Gy > 0) {
      
      phi = -PI/2;  // maybe.. check this later...
      
   } else {
   
     phi = atan((float)Gy/(float)Gz);
   }
   
   //De-rotating by the roll-angle:
   float bfy = (float)byp*cos(phi) - (float)bzp*sin(phi);
   bzp = (float)byp*sin(phi) + (float)bzp*cos(phi);
   gzp = (float)gyp*sin(phi) + (float)gzp*cos(phi);
   
   
   // calculating pitch angle Theta in radians:
   
   /*
   if( Gx > 0) {
      
      theta = Pi/2;
         
   } else if(Gx < 0) {
      
      theta = -Pi/2;
      
   } else {
   */
    //  float denominator = (float)gyp*sin(phi) + (float)gzp*cos(phi);
      theta = atan( (float)gxp*(-1)/( (float)gzp ));
   
   //}
   
   // De-rotate by pitch angle:
   
   float bfx = bxp*cos(theta) + bzp*sin(theta);
   float bfz = -bxp*sin(theta) + bzp*cos(theta);
   
   
   // Calculating yaw:
   
   if(bfz == 0) {
      psi = -Pi/2;
   }
   
   psi = atan( (-1)*(float)bfy / (float)bfx );
   
}
