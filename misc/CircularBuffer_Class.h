#ifndef _DISPLAY_BUFFER_H
#define _DISPLAY_BUFFER_H


template <class T, uint16_t S>
struct CircularBuffer { // note: this could be an STL array: template < class T, size_t N > class array;

    CircularBuffer(): head(0), top(S) {
      memset((void*)buffer, 0, S * sizeof(T));
    }

    CircularBuffer(const T &val): head(0), top(S) {
       for (uint16_t i=0; i<S; i++) buffer[i] = val;
    }

    CircularBuffer(const T &val, uint16_t _top): head(0) {
      top = constrain(_top, 1, S);
      for (uint16_t i=0; i<S; i++) buffer[i] = val;
    }

	inline void fill(const T &val) {
		 for (uint16_t i=0; i<S; i++) buffer[i] = val;
	}

    inline void setTop(uint16_t _top) { // reset content? No
      top = constrain(_top, 1, S);
	  head = constrain(head, 1, top-1);
    }

    inline void pushCircular (T val) {
      head = (head + 1) % top;
      buffer [head] = val;
    }

	inline void setOnHead(T& val) {
      buffer [head] = val;
    }

    inline void setRelative(uint16_t index, T& val) {
      buffer [index] = val;
    }

    T getOnHead () const {
      return buffer [head];
    }

    T getFromZero (uint16_t index) const { // same than operator: [index]
      // wrapping if too big:
      return buffer [index % top];
    }

    T getFromHead (uint16_t index) const {
      return buffer [(head + index) % top];
    }

    T getPreInc () {
      head = (head + 1) % top;
      return buffer [head];
    }

    T getPostInc () {
      T newPos(buffer [head]);
      head = (head + 1) % top;
      return newPos;
    }

    T operator[] (uint16_t index) const {
      return buffer [index % top];
    }

    bool isOnZero() {
      return (head == 0);
    }

    bool isOnEnd() {
      return (head == (top - 1));
    }

    inline void setHead(uint16_t _head) {
      head = _head;
    }

	inline void setHeadOnZero(uint16_t _head) {
      head = 0;
    }

	inline void setHeadOnTop(uint16_t _head) {
      head = top-1;
    }

  private:
    T buffer [S];
    uint16_t head, top;
};


#endif
