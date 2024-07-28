#ifndef __AKDYOLO_HPP__
#define __AKDYOLO_HPP__


// Boost
#include <boost/array.hpp>
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/detail/pair.hpp>
#include <boost/tuple/tuple.hpp>

namespace carlsimio { 

	// AKD Yolo Event -> (class(int),probability(double),direction(double),distance(double)) 
	// each class is mapped to a dedicated neuron group (e.g. 3x3 -> 27 20 
	// the doubles are scalar values to be mapped to a .. 2 precision is min, MVP is direction and distance, 
	// V 2: probability influences motivation, class are learned / good/bad => MVP 2 one target and others, 
	// target is a number 11 car
	// generic: int vector for several classification mapping, double vector for scalar mapping
	const unsigned int AkdYoloEventInts = 1;
	const unsigned int AkdYoloEventDoubles = 3; 
	typedef boost::array<int, AkdYoloEventInts> yolo_event_ints_t;	// int vector for classification mapping to distinct neurons
	typedef boost::array<double, AkdYoloEventDoubles> yolo_event_doubles_t;	
	typedef boost::tuple<yolo_event_ints_t, yolo_event_doubles_t> yolo_event_t;	
	typedef boost::container::vector<yolo_event_t> yolo_events_t;		// reader buffer
	// TradeEvent Ranges  -> initialized by parameter rather than an rpc request 
	enum ENUM_YOLO_EVENT_RANGE_TYPE {YOLO_EVENT_RANGE_MIN, YOLO_EVENT_RANGE_MAX};  
	typedef boost::array<yolo_event_t, 2> yolo_event_range_t;  // min/max
	typedef boost::container::vector<yolo_event_range_t> yolo_event_ranges_t; 

};


#endif // __AKDYOLO_HPP__