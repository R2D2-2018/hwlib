// ==========================================================================
//
// File      : hwlib-test.hpp
// Part of   : C++ hwlib library for close-to-the-hardware OO programming
// Copyright : wouter@voti.nl 2017
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// ==========================================================================

// this file contains Doxygen lines
/// @file

#ifndef HWLIB_TEST_H
#define HWLIB_TEST_H

#include HWLIB_INCLUDE( hwlib-all.hpp )

#include HWLIB_INCLUDE ( time.h )
#include HWLIB_INCLUDE ( stdint.h )
#include HWLIB_INCLUDE ( stdio.h )
#include HWLIB_INCLUDE ( initializer_list )
#include HWLIB_INCLUDE ( unistd.h )

#include HWLIB_INCLUDE ( chrono )
#include HWLIB_INCLUDE ( thread )

namespace test {

void wait_ns( int_fast32_t n );
void wait_us( int_fast32_t n );
void wait_ms( int_fast32_t n ); 
bool uart_char_available();
char uart_getc();
void uart_putc( char c );

#ifdef HWLIB_ONCE
bool now_ticks_begin_initialized = false;

uint64_t now_ticks(){
   static decltype(std::chrono::high_resolution_clock::now()) begin;

   if (!now_ticks_begin_initialized) {
      begin = std::chrono::high_resolution_clock::now();
      now_ticks_begin_initialized = true;
   }

   auto end = std::chrono::high_resolution_clock::now();
   return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}   

uint64_t ticks_per_us(){
   return 1;
}   

uint64_t now_us(){
   return now_ticks() / ticks_per_us();
}

void HWLIB_WEAK wait_us( int_fast32_t n ){ 
   std::this_thread::sleep_for(std::chrono::microseconds(n));
}

void HWLIB_WEAK wait_ms( int_fast32_t n ){
   std::this_thread::sleep_for(std::chrono::milliseconds(n));
}

void HWLIB_WEAK wait_ns( int_fast32_t n ){ 
   std::this_thread::sleep_for(std::chrono::nanoseconds(n));
}

void uart_putc( char c ){
    std::putc(c, stdout);
}

char uart_getc(){
   return std::getc(stdin);   
}

bool HWLIB_WEAK uart_char_available(){
   return 1;
}
#endif // HWLIB_ONCE

/// pin_in implementation for unit testing
template <int L = 256>
class pin_in : public hwlib::pin_in {
private:
   uint32_t read_i;
   uint8_t data[L];
   
public:

   /// \brief
   /// Test pin_in constructor
   /// \details
   /// Constructor for a testing pin.
   ///
   /// The provided arguments are all the ones and zeroes which this pin will
   /// return in series for every get() call.
   template <class... Args>
   pin_in(Args... lst) : read_i(0), data { lst... }
   {}
   
   bool get( 
      hwlib::buffering buf = hwlib::buffering::unbuffered 
   ) override {
      return data[read_i++];   
   }
};

/// pin_out implementation for unit testing
template <int L = 256>
class pin_out : public hwlib::pin_out {
private:
   uint32_t write_i;
   uint8_t data[L];
   
public:

   /// \brief
   /// Test pin_out constructor
   /// \details
   /// Constructor for a testing pin.
   pin_out() : write_i(0), data()
   {}
   
   void set( 
      bool v,
      hwlib::buffering buf = hwlib::buffering::unbuffered  
   ) override {
      data[write_i++] = v;
   }

   /// \brief
   /// Retrieves a list of all previously called set(v) calls.
   /// Where v is used as data.
   uint8_t* get_data() {
      return data;
   }
};

/// pin_in_out implementation for unit testing
template <int L_W = 256, int L_R = 256>
class pin_in_out : public hwlib::pin_in_out {
private:
   test::pin_in<L_R> test_pin_in;
   test::pin_out<L_W> test_pin_out;
   int8_t direction = -1; // undefined pin direction
   
public:

   /// \brief
   /// Test pin_in_out constructor
   /// \details
   /// Constructor for a testing pin.
   ///
   /// The provided arguments are all the ones and zeroes which this pin will
   /// return in series for every get() call.
   template <class... Args>
   pin_in_out(Args... lst) : test_pin_in { lst... }
   {}
   
   void direction_set_input() override {
      direction = 0;
   }
   
   bool get(
      hwlib::buffering buf = hwlib::buffering::unbuffered 
   ) override {
      if (direction == 0) {
         return test_pin_in.get();
      }
   }   
   
   void direction_set_output() override {
      direction = 1;
   }
   
   void set( 
      bool v,
      hwlib::buffering buf = hwlib::buffering::unbuffered 
   ) override {
      if (direction == 1) {
         test_pin_out.set(v);
      }
   }

   /// \brief
   /// Retrieves a list of all previously called set(v) calls.
   /// Where v is used as data.
   uint8_t* get_data() {
      return test_pin_out.get_data();
   }
};

/// pin_oc implementation for unit testing
template <int L_W = 256, int L_R = 256>
class pin_oc : public hwlib::pin_oc {
private:
   test::pin_in<L_R> test_pin_in;
   test::pin_out<L_W> test_pin_out;

public:

   /// \brief
   /// Test pin_oc constructor
   /// \details
   /// Constructor for a testing pin.
   ///
   /// The provided arguments are all the ones and zeroes which this pin will
   /// return in series for every get() call.
   template <class... Args>
   pin_oc(Args... lst) : test_pin_in { lst... }
   {}
   
   bool get(
      hwlib::buffering buf = hwlib::buffering::unbuffered 
   ) override {
       return test_pin_in.get();
   }   
   
   void set( 
      bool v,
      hwlib::buffering buf = hwlib::buffering::unbuffered 
   ) override {
      test_pin_out.set(v);
   }

   /// \brief
   /// Retrieves a list of all previously called set(v) calls.
   /// Where v is used as data.
   uint8_t* get_data() {
      return test_pin_out.get_data();
   }
};

/// pin_adc implementation for unit testing
template <int L = 256, int B = 12>
class pin_adc : public hwlib::adc {
private:
   uint32_t read_i;
   adc_value_type data[L];
   
public:

   /// \brief
   /// Test pin_oc constructor
   /// \details
   /// Constructor for a testing pin.
   ///
   /// The provided arguments are all the digital values which this pin will
   /// return in series for every get() call.
   template <class... Args>
   pin_adc(Args... lst) : hwlib::adc(B), read_i(0), data { lst... }
   {}

   adc_value_type get() override {
      return data[read_i++];
   }
};

}; // namespace test

namespace hwlib {

namespace target = ::test;
namespace test = ::test;

#ifdef HWLIB_ONCE

void HWLIB_WEAK uart_putc( char c ){
   target::uart_putc(c);
}

bool HWLIB_WEAK uart_char_available(){
   return 1;
}

char HWLIB_WEAK uart_getc( ){
   return target::uart_getc();
}

uint64_t now_ticks(){
   return target::now_ticks();
}   

uint64_t ticks_per_us(){
   return target::ticks_per_us();
}   

uint64_t now_us(){
   return target::now_us();
}   

void wait_us_busy( int_fast32_t n ){ 
   target::wait_us(n);
}

void wait_ms_busy( int_fast32_t n ){
   target::wait_ms(n);
}

void wait_ns_busy( int_fast32_t n ){ 
   target::wait_ns(n);
}

void HWLIB_WEAK wait_us( int_fast32_t n ){ 
   target::wait_us(n);
}

void HWLIB_WEAK wait_ms( int_fast32_t n ){
   target::wait_ms(n);
}

void HWLIB_WEAK wait_ns( int_fast32_t n ){ 
   target::wait_ns(n);
}

#endif

}; //namespace hwlib 

#endif // HWLIB_TEST_H
