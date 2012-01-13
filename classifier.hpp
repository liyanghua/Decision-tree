#ifndef _CLASSIFIER_HPP_
#define _CLASSIFIER_HPP_

#include "inst_bag.hpp"

class Classifier {
    public:
        virtual void train(const string& file) {}
        virtual int classify(Instance& inst) { return -1; }

        virtual InstanceBag* get_bag() { return NULL;}

        virtual ~Classifier() {}
};
#endif
