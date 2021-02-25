

inline int16_t adaptValue(uint16_t old_max, int16_t x){
  
  if(old_max > THRESH_1  &&  old_max <= THRESH_2)  return ((x * 9) >> FIXED_POINT_SHIFT);
  if(old_max > THRESH_2  &&  old_max <= THRESH_3)  return ((x * 6) >> FIXED_POINT_SHIFT);
  if(old_max > THRESH_3  &&  old_max <= THRESH_4)  return ((x * 5) >> FIXED_POINT_SHIFT);
  if(old_max > THRESH_4)                          return ((x * 4) >> FIXED_POINT_SHIFT);
  
  return x; // if all conditions fail, do not modify value
}


  // Clamps a value between two boundaries
inline uint8_t _clamp(int16_t val, uint16_t lowerb, uint16_t upperb){
  if(val > upperb)  return upperb;
  if(val < lowerb)  return lowerb;
  return val;
}
