float atan2(float y,float x) {
   float v;
   if (x>0) {
      v=atan(y/x);
   }

   if (y>=0 && x<0) {
    v=pi+atan(y/x);
   }

   if (y<0 && x<0) {
    v=-pi+atan(y/x);
   }

   if (y>0 && x==0) {
    v=pi/2;
   }

   if (y<0 && x==0) {
    v=-pi/2;
   }

   if (v<0) {
    v=v+2*pi;
   }
   
   return v;
}
