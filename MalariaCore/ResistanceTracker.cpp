/* 
 * File:   ResistanceTracker.cpp
 * Author: Merlin
 * 
 * Created on September 24, 2013, 10:00 AM
 */

#include "ResistanceTracker.h"
#include "Model.h"
#include "Config.h"
#include "Genotype.h"
#include "ParasiteDatabase.h"
#include <numeric>
#include <boost/foreach.hpp>

template <typename Iterator>
inline bool next_combination(const Iterator first, Iterator k, const Iterator last) {
    /* Credits: Thomas Draper */
    if ((first == last) || (first == k) || (last == k))
        return false;
    Iterator itr1 = first;
    Iterator itr2 = last;
    ++itr1;
    if (last == itr1)
        return false;
    itr1 = last;
    --itr1;
    itr1 = k;
    --itr2;
    while (first != itr1) {
        if (*--itr1 < *itr2) {
            Iterator j = k;
            while (!(*itr1 < *j)) ++j;
            std::iter_swap(itr1, j);
            ++itr1;
            ++j;
            itr2 = k;
            std::rotate(itr1, j, last);
            while (last != j) {
                ++j;
                ++itr2;
            }
            std::rotate(k, itr2, last);
            return true;
        }
    }
    std::rotate(first, k, last);
    return false;
}

ResistanceTracker::ResistanceTracker() {
}

ResistanceTracker::ResistanceTracker(const ResistanceTracker& orig) {
}

ResistanceTracker::~ResistanceTracker() {
}

void make_resistance_profile(std::vector<int>& vResistanceID, std::vector<int>& drugs_used, DrugTypePtrMap& drug_db, const int& size) {
    vResistanceID.clear();
    int new_size = (drugs_used.size() < size) ? drugs_used.size() : size;
    do {
        boost::dynamic_bitset<> bs(drug_db.begin()->second->loci_that_confer_resistance().size(), 0ul);

        for (IntVector::iterator it = drugs_used.begin(); it != (drugs_used.begin() + new_size); it++) {
            bs |= drug_db[*it]->loci_that_confer_resistance();
            //            std::cout << bs << std::endl;
        }
        vResistanceID.push_back(bs.to_ulong());
        //        std::cout << vResistanceID.size() << std::endl;

        //            int id = accumulate(vDrugs.begin(), vDrugs.begin() + size, 0);
        //        vResistanceID.push_back(id);
        //        assert(NumberOfSetBits(accumulate(drugs_used.begin(), drugs_used.begin() + size, 0)) == size);

    } while (next_combination(drugs_used.begin(), drugs_used.begin() + new_size, drugs_used.end()));
}

void make_arterminsinin_resistance_profile(std::vector<int>& vResistanceID, std::vector<int>& drugs_used, DrugTypePtrMap& drug_db) {
    //    vResistanceID.push_back(1);
    //    if (drugs_used.size() < 2) {
    //        //        vResistanceID.push_back(1);
    //        return;
    //    }

    //    do {
    //        boost::dynamic_bitset<> bs(drug_db.begin()->second->resistance_bit_string().size(), 0ul);
    //
    //        for (IntVector::iterator it = drugs_used.begin(); it != (drugs_used.begin() + 2); it++) {
    //            bs |= drug_db[*it]->resistance_bit_string();
    //            //            std::cout << bs << std::endl;
    //        }
    //
    //        if (bs[0]) {
    //            vResistanceID.push_back(bs.to_ulong());
    ////            std::cout << bs.to_ulong() << std::endl;
    //
    //        }
    //
    //
    //        //            int id = accumulate(vDrugs.begin(), vDrugs.begin() + size, 0);
    //        //        vResistanceID.push_back(id);
    //        //        assert(NumberOfSetBits(accumulate(drugs_used.begin(), drugs_used.begin() + size, 0)) == size);
    //
    //    } while (next_combination(drugs_used.begin(), drugs_used.begin() + 2, drugs_used.end()));




    //     std::cout << drug_db.size() << std::endl;



    for (GenotypePtrMap::iterator it = Model::CONFIG->parasite_db()->genotype_db().begin(); it != Model::CONFIG->parasite_db()->genotype_db().end(); it++) {
        if (it->second->resistance_bit_string()[0]) {
            //            std::cout << it->first << std::endl;
            vResistanceID.push_back(it->first);
            //            std::cout << it->first << std::endl;

        }
    }


}

void ResistanceTracker::initialize() {
    parasite_population_count_.assign(Model::CONFIG->number_of_parasite_types(), 0);

    DrugTypePtrMap& drug_db = Model::CONFIG->drug_db()->drug_db();
    IntVector drugs_used;

    boost::dynamic_bitset<> bs(drug_db.begin()->second->loci_that_confer_resistance().size(), 0ul);
    for (DrugTypePtrMap::iterator it = drug_db.begin(); it != drug_db.end(); it++) {
        drugs_used.push_back(it->first);
        bs |= it->second->loci_that_confer_resistance();
    }
    all_resistance_id_ = bs.to_ulong();
    //    
    make_resistance_profile(single_resistance_ids_, drugs_used, drug_db, 1);
    make_resistance_profile(double_resistance_ids_, drugs_used, drug_db, 2);
    make_resistance_profile(tripple_resistance_ids_, drugs_used, drug_db, 3);
    make_resistance_profile(quadruple_resistance_ids_, drugs_used, drug_db, 4);

    make_arterminsinin_resistance_profile(artemisinin_ids_, drugs_used, drug_db);

    total_resistance_ids_.clear();
    total_resistance_ids_.push_back(&single_resistance_ids_);
    total_resistance_ids_.push_back(&double_resistance_ids_);
    total_resistance_ids_.push_back(&tripple_resistance_ids_);
    total_resistance_ids_.push_back(&quadruple_resistance_ids_);

    //    std::cout << quadruple_resistance_ids_.size() << std::endl;
    //    std::cout << quadruple_resistance_ids_[0] << std::endl;

    //    BOOST_FOREACH(int id, single_resistance_ids_) {
    //        std::cout << id << std::endl;
    //    }
    //    std::cout << "===" << std::endl;
    //
    //    BOOST_FOREACH(int id, double_resistance_ids_) {
    //        std::cout << id << std::endl;
    //    }
    //    std::cout << "===" << std::endl;
    //
    //    BOOST_FOREACH(int id, tripple_resistance_ids_) {
    //        std::cout << id << std::endl;
    //    }
    //    std::cout << "===" << std::endl;
    //
    //    BOOST_FOREACH(int id, quadruple_resistance_ids_) {
    //        std::cout << id << std::endl;
    //    }
    //
    //    std::cout << "===" << std::endl;
    //    std::cout << total_resistance_id_<< std::endl;

    any_single_tracking_time_.clear();
    all_single_tracking_time_.clear();
    any_double_tracking_time_.clear();
    all_double_tracking_time_.clear();
    any_triple_tracking_time_.clear();
    all_triple_tracking_time_.clear();
    any_quadruple_tracking_time_.clear();
    all_quadruple_tracking_time_.clear();
    total_tracking_time_.clear();
    artemisinin_tracking_time_.clear();


    tracking_values_.clear();
    tracking_values_.push_back(0.001);
    tracking_values_.push_back(0.005);
    tracking_values_.push_back(0.01);
    tracking_values_.push_back(0.05);
    tracking_values_.push_back(0.10);
    tracking_values_.push_back(0.25);

    for (int i = 0; i < tracking_values_.size(); i++) {
        any_single_tracking_time_.push_back(-1);
        all_single_tracking_time_.push_back(-1);
        any_double_tracking_time_.push_back(-1);
        all_double_tracking_time_.push_back(-1);
        any_triple_tracking_time_.push_back(-1);
        all_triple_tracking_time_.push_back(-1);
        any_quadruple_tracking_time_.push_back(-1);
        all_quadruple_tracking_time_.push_back(-1);
        total_tracking_time_.push_back(-1);
        artemisinin_tracking_time_.push_back(-1);
    }
}

void ResistanceTracker::increase(const int& id) {
    parasite_population_count_[id] += 1;
    total_ += 1;

}

void ResistanceTracker::decrease(const int& id) {
    parasite_population_count_[id] -= 1;
    total_ -= 1;
}

void ResistanceTracker::change(const int& from, const int& to) {
    parasite_population_count_[from] -= 1;
    parasite_population_count_[to] += 1;
}

void ResistanceTracker::update_resistance_tracker() {

    for (int i = 0; i < tracking_values_.size(); i++) {
        if (any_single_tracking_time_[i] == -1) {
            update_time_value(any_single_tracking_time_[i], max_fraction_resistance(single_resistance_ids_), tracking_values_[i]);
        }
        if (all_single_tracking_time_[i] == -1) {
            update_time_value(all_single_tracking_time_[i], min_fraction_resistance(single_resistance_ids_), tracking_values_[i]);
        }
        if (any_double_tracking_time_[i] == -1) {
            update_time_value(any_double_tracking_time_[i], max_fraction_resistance(double_resistance_ids_), tracking_values_[i]);
        }
        if (all_double_tracking_time_[i] == -1) {
            update_time_value(all_double_tracking_time_[i], min_fraction_resistance(double_resistance_ids_), tracking_values_[i]);
        }

        if (any_triple_tracking_time_[i] == -1) {
            update_time_value(any_triple_tracking_time_[i], max_fraction_resistance(tripple_resistance_ids_), tracking_values_[i]);
        }
        if (all_triple_tracking_time_[i] == -1) {
            update_time_value(all_triple_tracking_time_[i], min_fraction_resistance(tripple_resistance_ids_), tracking_values_[i]);
        }
        if (any_quadruple_tracking_time_[i] == -1) {
            update_time_value(any_quadruple_tracking_time_[i], max_fraction_resistance(quadruple_resistance_ids_), tracking_values_[i]);
        }
        if (all_quadruple_tracking_time_[i] == -1) {
            update_time_value(all_quadruple_tracking_time_[i], min_fraction_resistance(quadruple_resistance_ids_), tracking_values_[i]);
        }

        if (total_tracking_time_[i] == -1) {

            // check how many drug-used in the simulation
            int size = Model::CONFIG->parasite_db()->get(0)->resistance_bit_string().size();

            //get weighted sum
            total_resistance_frequency_ = 0;
            for (int c = 0; c < size; c++) {
                double sub_sum = 0;
                if (c == size - 1) {
                    sub_sum += parasite_population_count_[all_resistance_id_];
//                    std::cout << << "Hello" << std::endl;
                } else {
                    for (int j = 0; j < total_resistance_ids_[c]->size(); j++) {
                        sub_sum += parasite_population_count_[total_resistance_ids_[c]->at(j)];
                    }
                }

                total_resistance_frequency_ += (c + 1) * sub_sum;
            }

            total_resistance_frequency_ = (total_resistance_frequency_ / total_) / size;


            update_time_value(total_tracking_time_[i], total_resistance_frequency_, tracking_values_[i]);
        }


        if (artemisinin_tracking_time_[i] == -1) {
            update_time_value(artemisinin_tracking_time_[i], sum_fraction_resistance(artemisinin_ids_), tracking_values_[i]);
        }
    }
}

double ResistanceTracker::max_fraction_resistance(const IntVector& resitance_ids) {
    double max = 0;

    for (int i = 0; i < resitance_ids.size(); i++) {
        //        double fraction = (total_ == 0) ? 0 : parasite_population_count_[resitance_ids[i]] / (double) total_;
        if (parasite_population_count_[resitance_ids[i]] > max) {
            max = parasite_population_count_[resitance_ids[i]];
        }
    }
    max = (total_ == 0) ? 0 : max / (double) total_;
    return max;
}

double ResistanceTracker::min_fraction_resistance(const IntVector& resitance_ids) {
    double min = 1;

    for (int i = 0; i < resitance_ids.size(); i++) {
        if (parasite_population_count_[resitance_ids[i]] < min) {
            min = parasite_population_count_[resitance_ids[i]];
        }
    }
    min = (total_ == 0) ? 0 : min / (double) total_;
    return min;
}

double ResistanceTracker::sum_fraction_resistance(const IntVector& resitance_ids) {
    double sum = 0;

    for (int i = 0; i < resitance_ids.size(); i++) {
        sum += parasite_population_count_[resitance_ids[i]];
    }
    sum = (total_ == 0) ? 0 : sum / (double) total_;
    return sum;
}

void ResistanceTracker::update_time_value(int& tracking_time, const double& value, const double& check_value) {

    if (tracking_time == -1) {
        if (value >= check_value) {
            tracking_time = Model::SCHEDULER->current_time();
        }
    }
}