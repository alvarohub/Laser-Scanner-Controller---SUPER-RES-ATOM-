/*
    DSP.h - Arduino DSP mini library
    Copyright (C) 2015-2018 Pablo Gindel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    VERSION:
          April 2018: library modified by Alvaro Cassinelli
*/


#ifndef DSP_H
#define DSP_H

#include <Arduino.h>


///////////////////////////////////////////////
//         ring buffer (con template)        //
///////////////////////////////////////////////

template <typename T, int S>
struct CircularBuffer {

    CircularBuffer(): pos(0) {
      memset((void*)buffer, 0, S * sizeof(T)); //NOTE: memset only sets the memory with an 
      // integer: it cannot do it with a generic object (for that we should use STL containers, or of course a for loop...)
      topSize = S;
    }

    CircularBuffer(const T& val): pos(0) {
      memset((void*)buffer, 0, S * sizeof(T)); // no real need...  
      topSize=S;
       for (uint16_t i=0; i<topSize; i++) {
        buffer[i] = val;
      }
    }
    
    CircularBuffer(const T& val, uint16_t _topSize): pos(0) {
      memset((void*)buffer, 0, S * sizeof(T)); // no real need...
      topSize = constrain(_topSize, 1, S);
       for (uint16_t i=0; i<topSize; i++) {
        buffer[i] = val;
      }
    }

    void resizeTop(uint16_t _topSize) { // reset content?
      topSize = constrain(_topSize, 1, S);
      pos = 0;
    }

    void store (T value) { 
      pos = (pos + 1) % topSize;
      buffer [pos] = value;
    }

    void set(uint16_t index, T& val) {
      buffer [index] = val;
    }

    void set(T& value) {
      buffer [pos] = value;
    }

    T get () const {
      return buffer [pos];
    }

    T get (uint16_t index) const { // same than operator: [index]
      // wrapping if too big:
      return buffer [index % topSize];
    }
    
    T getRel (uint16_t index) const {
      return buffer [(pos + index) % topSize];
    }

    T getPreInc () {
      pos = (pos + 1) % topSize;
      return buffer [pos];
    }

    T getPostInc () {
      T newPos(buffer [pos]);
      pos = (pos + 1) % topSize;
      return newPos;
    }

    T operator[] (uint16_t index) const {
      // wrapping if too big:
      return buffer [index % topSize];
    }

    bool isInStart() {
      return (pos == 0);
    }

    bool isInEnd() {
      return (pos == (topSize - 1));
    }

    void setPos(uint16_t _posHead) {
      pos = _posHead;
    }

  private:
    T buffer [S];
    uint16_t pos, topSize;
};


typedef CircularBuffer <float, 3> RBF3;


//////////////////////////////////////////////////////////
//                  filtros recursivos                  //
//////////////////////////////////////////////////////////

// http://www.dspguide.com/ch19/2.htm


enum FilterType {LPF, HPF};

class SinglePole {

  public:

    SinglePole (FilterType type, float cutoff) {           // cutoff entre 0 y 0.5 con respecto a fs
      computeCoefficients(type, cutoff);
    }

    SinglePole (FilterType type, float fs, float fc) {     // fs y fc expresadas en Hz
      computeCoefficients(type, fc / fs);
    }

    float update (float value) {
      input_.store (value);
      output_.store (a0 * input_.get(0) + a1 * input_.get(1) + b1 * output_.get(0));
      return output_.get(0);
    }

  private:

    float a0, a1, b1;                      // coeficientes
    RBF3 input_, output_;                  // ring buffers

    void computeCoefficients(FilterType type, float cutoff) {           // cutoff entre 0 y 0.5
      // assert (cutoff<0.5 && cutoff>0);
      // calcula los coeficientes
      b1 = exp (-2 * PI * cutoff);
      switch (type) {
        case LPF:
          a0 = 1 - b1;
          a1 = 0;
          break;
        case HPF:
          a0 = (1 + b1) / 2.0;
          a1 = -a0;
          break;
      }
    }

};


///////////////////////////////////////////////////////////////////
//                     moving average filter                     //
///////////////////////////////////////////////////////////////////

template <typename T, int S>
class MovAvgFilter {
  public:
    T process (T new_value) {
      filter_value -= rb.get(S - 1);
      rb.store (new_value);
      filter_value += rb.get(0);
      return filter_value;
    }
  private:
    CircularBuffer <T, S> rb;
    T filter_value;
};



#endif
