#ifndef _INST_BAG_
#define _INST_BAG_

#include <fstream>
#include <assert.h>
#include <cmath>

#include "type_def.hpp"
const static char* NAME = ".name";
const static char* DATA = ".data";

namespace util {
    void lower(string& s) {
        for (size_t i=0; i<s.size(); ++i) {
            s[i] = tolower(s[i]);
        }
    }

    void trim(string& str) {
        string::size_type pos = str.find_last_not_of(' ');
        if(pos != string::npos)
        {
            str.erase(pos + 1);
            pos = str.find_first_not_of(' ');
            if(pos != string::npos) str.erase(0, pos);
        }
        else str.erase(str.begin(), str.end());
    }

    template <class Key, class Val>
        void dump_map(const map<Key, Val>& m) {
            typename map<Key, Val>::const_iterator it(m.begin());
            for (; it != m.end(); ++it) {
                cout << it->first << "->" << it->second << endl;
            }
        }

    template <class Type>
        void dump_vector(const vector<Type>& v) {
            typename vector<Type>::const_iterator it(v.begin());
            for (; it != v.end(); ++it) {
                cout << *it << " ";
            }
            cout << endl;
        }
};

using namespace util;

class Instance {
    private:
        bool training;
        // only store the index
        vector<int> attr_val_vec;
        int label_index;

        int find_attr_val_index(const vector<AttrName>& attr_name_vec, int name_index, const string& attr_val) {
            assert(name_index >= 0 && name_index < (int)attr_name_vec.size());

            const AttrName& attrName = attr_name_vec[name_index];
            for(size_t i=0; i<attrName.vals.size(); ++i) {
                if (attr_val.compare(attrName.vals[i]) == 0) {
                    return i;
                }
            }
            return -1;
        }
    public:
        Instance() : training(true), label_index(-1) { attr_val_vec.clear(); }
        int get_label_index() {
            return label_index;
        }
        const vector<int>& get_attr_val_vec() {
            return attr_val_vec;
        }

        bool contains(int attr_name_index, int attr_val_index) {
            assert(attr_name_index >=0 && attr_name_index < (int)attr_val_vec.size());

            return (attr_val_vec[attr_name_index] == attr_val_index);
        }

        void dump(const vector<AttrName>& attr_name_vec) {
            // 0, 1 
            cout << "Label: " << label_index << endl;

            size_t len = attr_val_vec.size();
            cout << "len:" << len << endl;
            for (size_t i=0; i<len; ++i) {
                const AttrName& m = attr_name_vec[i];
                int val_index = attr_val_vec[i];
                cout << m.name << "-->" << m.vals[val_index] << endl;
            }
            cout << endl;
        }

        void init(const string& line, 
                const map<Label, int>& m,
                const vector<AttrName>& attr_name_vec) {
            int start_pos = 0;
            int name_index = 0;
            while(true) {
                int pos = line.find(',', start_pos);
                if (pos == -1) {
                    string tmp(line.substr(start_pos));
                    trim(tmp);
                    lower(tmp);
                    label_index = m.at(tmp);
                    break;

                }
                else {
                    string tmp(line.substr(start_pos, pos - start_pos));
                    trim(tmp);
                    lower(tmp);
                    int attr_val_index = find_attr_val_index(attr_name_vec, name_index, tmp);
                    attr_val_vec.push_back(attr_val_index);
                    start_pos = pos + 1;
                }
                name_index += 1;
            }
        }

};

class InstanceBag {
    private:
        map<Label, int> label_map;
      
        vector<AttrName> attr_name_vec;
        vector<Instance> inst_vec;

        // the instance number for each category
        map<int, int> label_counts;

    private:
        void read_attr(const string& file) {
            ifstream in(file.c_str());
            if (in.is_open()) {
                string line;
                int line_num = 0;
                while(in.good()) {
                    getline(in, line);
                    if (line.empty()) continue;
                    if (line_num == 0) { // read the label
                        int start_pos = 0;
                        int label_index = 0;
                        while(true) {
                            int pos = line.find(',', start_pos);
                            if (pos == -1) {
                                string tmp(line.substr(start_pos));
                                trim(tmp);
                                lower(tmp);
                                label_map[tmp] = label_index;
                                break;

                            }
                            else {
                                string tmp(line.substr(start_pos, pos - start_pos));
                                trim(tmp);
                                lower(tmp);
                                label_map[tmp] = label_index;
                                start_pos = pos + 1;
                            }
                            label_index += 1;
                        }
                    }
                    else { // read the name
                        int pos = line.find(':');
                        if (pos == -1) {
                            continue;
                        }
                        string name(line.substr(0, pos));
                        AttrName attrName(ATTR_NAME_DISCRETE, name);

                        string value(line.substr(pos + 1));
#if 0
                        cout << "name:" << name << ",value:" << value << endl;
#endif 
                        int start_pos = 0;
                        while(true) {
                            int pos = value.find(',', start_pos);
                            if (pos == -1) {
                                string tmp(value.substr(start_pos));
                                trim(tmp);
                                lower(tmp);
                                attrName.vals.push_back(tmp);
                                break;

                            }
                            else {
                                string tmp(value.substr(start_pos, pos - start_pos));
                                trim(tmp);
                                lower(tmp);
                                attrName.vals.push_back(tmp);
                                start_pos = pos + 1;
                            }

                        }

                        attr_name_vec.push_back(attrName);

                    }
                    line_num += 1;
                }
            }
            in.close();
        }

        void read_data(const string& file) {
            ifstream in(file.c_str());
            if (in.is_open()) {
                string line;
                while(in.good()) {
                    getline(in, line);
                    if (line.empty()) continue;
                    Instance inst;
                    inst.init(line, label_map, attr_name_vec);
                    label_counts[inst.get_label_index()] += 1;
#if 0
                    inst.dump(attr_name_vec);
#endif
                    inst_vec.push_back(inst);
                }
            }
            in.close();
        }

    public:
        void load_data(const string& file) {
            read_attr(file + NAME);
            read_data(file + DATA);     
        }

        int get_attr_name_num() {
            return attr_name_vec.size();
        }

        void get_label_string(int i, string& label) {
            map<Label, int>::iterator it(label_map.begin());
            for(; it != label_map.end(); ++it) {
                if (it->second == i) {
                    label.assign(it->first);
                }
            }
        }

        double get_beta_entropy() {
            double E = 0;
            double total = inst_vec.size();
            map<int, int>::iterator it(label_counts.begin());
            for(; it != label_counts.end(); ++it) {
                double p = (double)it->second / total;
                E += (-p) * log2(p);
            }
            
            return E;
        }

        Instance& operator[](size_t i) {
            assert(i >=0 && i < inst_vec.size());
            assert(!inst_vec.empty());

            return inst_vec[i];
        }

        const vector<AttrName>& get_attr_name_vec() {
            return attr_name_vec;
        }

        const string& get_attr_name(size_t i) {
            assert( i>= 0 && i < attr_name_vec.size());

            return attr_name_vec[i].name;
        }

        const string& get_attr_val(size_t i, size_t j) {
            assert( i>= 0 && i < attr_name_vec.size());
            assert( j>= 0 && j < attr_name_vec[i].vals.size());
            return attr_name_vec[i].vals[j];
        }


        size_t size() {
            return inst_vec.size();
        }
};


#endif
