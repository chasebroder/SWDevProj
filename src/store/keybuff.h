#pragma once

#include "key.h"
#include "../utils/object.h"
#include "../utils/string.h"

/**
 * A class analogous to StringBuffer but for Keys. The Key is external.
 * Provided as part of Milestone 4.
 *
 */
class KeyBuff : public Object {                                                  
  public:                                                                        
  Key* orig_; // external                                                        
  StrBuff buf_;                                                                  
                                                                                 
  KeyBuff(Key* orig) : orig_(orig), buf_(*orig) {}                               
                 
  // Methods that append to the key.				 
  KeyBuff& c(String &s) { buf_.c(s); return *this;  }                            
  KeyBuff& c(size_t v) { buf_.c(v); return *this; }                              
  KeyBuff& c(const char* v) { buf_.c(v); return *this; }                         
                   
  /** Get the key we've constructed in this buffer */  
  Key* get() {                                                                   
    String* s = buf_.get();                                                      
    buf_.c(orig_->c_str());                                                      
    Key* k = new Key(s->steal(), orig_->home());                                 
    delete s;                                                                    
    return k;                                                                    
  }                                                                              
}; // KeyBuff 