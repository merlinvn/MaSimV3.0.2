/* 
 * File:   Scheduler.cpp
 * Author: nguyentran
 * 
 * Created on March 22, 2013, 2:27 PM
 */

#include "Scheduler.h"
#include "Event.h"
#include "HelperFunction.h"
#include "Dispatcher.h"
#include "Model.h"
#include "Config.h"
#include "Population.h"
#include "ModelDataCollector.h"
#include "Strategy.h"
#include "TMEScheduler.h"
#include "ImportationPeriodicallyEvent.h"
#include <boost/foreach.hpp>

Scheduler::Scheduler(Model* model) :
model_(model), total_time_(-1), current_time_(-1), is_force_stop_(false) {


}

Scheduler::Scheduler(const Scheduler& orig) {
}

Scheduler::~Scheduler() {

    BOOST_FOREACH(EventPtrVector events_list, timed_events_list_) {

        BOOST_FOREACH(Event* event, events_list) {
            DeletePointer<Event>(event);
        }
        events_list.clear();
    }
    timed_events_list_.clear();
}

void Scheduler::clear_all_events() {

    BOOST_FOREACH(EventPtrVector events_list, timed_events_list_) {

        BOOST_FOREACH(Event* event, events_list) {
            if (event->dispatcher() != NULL) {
                event->dispatcher()->remove(event);
            }
            DeletePointer<Event>(event);
        }
        events_list.clear();
    }
    timed_events_list_.clear();
}

int Scheduler::total_time() const {
    return total_time_;
}

void Scheduler::set_total_time(const int& value) {
    if (total_time_ > 0) {
        clear_all_events();
    }

    total_time_ = value;
    timed_events_list_.assign(total_time_, EventPtrVector());
}

void Scheduler::schedule(Event* event) {
    //TODO:: implement exception here 
    //1. Compare current time with event time
    //2. Event time cannot exceed total time
    timed_events_list_[event->time()].push_back(event);
    event->set_scheduler(this);
}

void Scheduler::cancel(Event* event) {
    event->set_executable(false);
}

void Scheduler::run() {
    current_time_ = 0;
    for (current_time_ = 0; !can_stop(); current_time_++) {
        //        std::cout << "Day: " << current_time_ << std::endl;
        begin_time_step();
        int size = timed_events_list_[current_time_].size();
        for (int i = 0; i < size; i++) {
            timed_events_list_[current_time_][i]->perform_execute();
            DeletePointer<Event>(timed_events_list_[current_time_][i]);
        }

        timed_events_list_[current_time_].clear();
        //actual release memory
        std::vector< Event* >(timed_events_list_[current_time_]).swap(timed_events_list_[current_time_]);

        end_time_step();
    }
}

void Scheduler::begin_time_step() {


    if (model_ != NULL) {
        Model::DATA_COLLECTOR->begin_time_step();
        //        model_->report_begin_of_time_step();

        model_->perform_infection_event();
    }

}

void Scheduler::end_time_step() {


    Model::POPULATION->perform_birth_event();
    Model::POPULATION->perform_death_event();
    Model::EXTERNAL_POPULATION->perform_death_event();

    ///for safety remove all dead by calling perform_death_event
    Model::POPULATION->perform_circulation_event();

    Model::POPULATION->perform_moving_to_external_population_event();

    //TODO:: TME Scheduler check and perform TMA Action(current_time_)

    Model::TME_SCHEDULER->check_and_perform_TME_Actions();

    update_end_of_time_step();

    report_end_of_time_step();
}

void Scheduler::update_end_of_time_step() {
    //update / calculate daily UTL
    Model::DATA_COLLECTOR->end_of_time_step();

    //update force of infection
    update_force_of_infection();


    //update resistance tracker

    //update person index to reduce memory
    //    if (current_time_ % Model::CONFIG->update_frequency() == 0) {
    //        Model::POPULATION->update();
    //    }
    Model::DATA_COLLECTOR->update_every_year();

    //check to switch strategy
    Model::CONFIG->strategy()->check_and_switch_therapy();
}

void Scheduler::report_end_of_time_step() {
    model_->report_end_of_time_step();
}

bool Scheduler::can_stop() {
    //TODO: put other criteria here
    //    std::cout << current_time_ << "\t" << Model::CONFIG->total_time() << std::endl;

    return (current_time_ > Model::CONFIG->total_time()) || Model::POPULATION->has_0_case() || is_force_stop_;
    //    return (current_time_ > Model::CONFIG->total_time()) ||  is_force_stop_;
}

void Scheduler::initialize(const int& total_time) {
    set_total_time(total_time);
    set_current_time(0);
}

void Scheduler::update_force_of_infection() {

    for (int loc = 0; loc < Model::CONFIG->number_of_locations(); loc++) {
        for (int pType = 0; pType < Model::CONFIG->number_of_parasite_types(); pType++) {
            Population* population = model_->population();
            population->force_of_infection_for7days_by_location_parasite_type()[current_time_ % Model::CONFIG->number_of_tracking_days()][loc][pType] = population->current_force_of_infection_by_location_parasite_type()[loc][pType];
        }
    }
}

int Scheduler::current_day_in_year() {
    return (current_time_ - Model::CONFIG->start_collect_data_day()) % 365;
}