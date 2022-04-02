/*
 * @Author: LinXuan
 * @Date: 2022-03-31 19:24:12
 * @Description:
 * @LastEditors: xv_rong
 * @LastEditTime: 2022-04-01 18:41:40
 * @FilePath: /FDO/CodeCraft-2022/src/data.cpp
 */
#include "data.h"

/*read_file的工具函数，读取一行文件流并按照指定分隔符分隔，返回分割后的vector<string>*/
vector<string> get_split_line(ifstream &file, char delim) {
    // 读取文件流的一行，使用delim分割
    vector<string> result;
    string line;

    file >> line;
    istringstream sin(line);

    while (getline(sin, line, delim)) {
        result.push_back(line);
    }

    return result;
}

/*从文件中获取数据，储存到一个Data结构体中并返回*/
Data read_file() {
    Data data;
    ifstream file;
    vector<string> split_line;

    /*读取config.ini*/
    file.open(INPUT + "config.ini");
    split_line = get_split_line(file, '\n'); // 忽略首行
    split_line = get_split_line(file, '=');  // qos_constraint
    data.qos_constraint = stoi(split_line[1]);
    split_line = get_split_line(file, '='); //  base_cost
    data.base_cost = stoi(split_line[1]);
    file.close();

    /*读取demand.csv*/
    file.open(INPUT + "demand.csv");
    // customer_site及其逆映射
    split_line = get_split_line(file, ','); // 首行，mtime+stream_id+...customer_site
    split_line.erase(split_line.begin());
    split_line.erase(split_line.begin());
    data.customer_site = split_line;
    for (size_t i = 0; i < data.customer_site.size(); i++) {
        data.re_customer_site[data.customer_site[i]] = i;
    }

    // stream_id与demand
    // 没有说时间按顺序给出，留一个 mtime -> index  的临时映射处理
    unordered_map<string, size_t> re_mtime_index;
    while (file.eof() == false) {
        split_line = get_split_line(file, ',');
        if (split_line.empty())
            break;
        // 处理m_time的映射
        string smtime = split_line[0];
        size_t m_time;
        if (re_mtime_index.count(smtime) == 0) {
            m_time = re_mtime_index.size();
            re_mtime_index[smtime] = m_time;
            data.demand.push_back(vector<vector<pair<int, int>>>(data.customer_site.size()));
        }

        string sstream_type = split_line[1];
        int stream_type;
        // 处理stream_type及其映射
        if (data.re_stream_type.count(sstream_type) == 0) {
            stream_type = data.stream_type.size();
            data.stream_type.push_back(sstream_type);
            data.re_stream_type[sstream_type] = stream_type;
        } else {
            stream_type = data.re_stream_type[sstream_type];
        }

        for (size_t i = 2; i < split_line.size(); i++) {
            data.demand[m_time][i - 2].push_back(make_pair(stoi(split_line[i]), stream_type));
        }
    }
    file.close();

    /*读取site_bandwith.csv*/
    file.open(INPUT + "site_bandwidth.csv");
    split_line = get_split_line(file, ','); // 忽略掉第一行
    while (file.eof() == false) {
        split_line = get_split_line(file, ',');
        if (split_line.empty())
            break;
        data.site_bandwidth.push_back(stoi(split_line[1]));
        data.edge_site.push_back(split_line[0]);
        data.re_edge_site[split_line[0]] = data.edge_site.size() - 1;
    }
    file.close();

    /*读取qos.csv*/
    file.open(INPUT + "qos.csv");
    // 预处理映射
    vector<string> tmp_customer_site = get_split_line(file, ',');
    tmp_customer_site.erase(tmp_customer_site.begin());
    vector<int> tmp_customer_list;
    for (auto &item : tmp_customer_site) {
        tmp_customer_list.push_back(data.re_customer_site[item]);
    }

    data.qos = vector<vector<int>>(data.customer_site.size(), vector<int>(data.edge_site.size(), 0));

    while (file.eof() == false) {
        split_line = get_split_line(file, ',');
        if (split_line.empty())
            break;
        int edge_index = data.re_edge_site[split_line[0]];
        for (size_t i = 1; i < split_line.size(); ++i) {
            data.qos[tmp_customer_list[i - 1]][edge_index] = stoi(split_line[i]);
        }
    }

    file.close();

    return data;
}

/*按照规定格式输出一个分配好的distribution*/
void output_distribution(const Data &data, const Distribution &distribution) {
    /*Distribution类型 [mtime][customer][...] = <edge_site, stream_type>*/
    ofstream fout(OUTPUT + "solution.txt");
    // 遍历时刻
    for (const auto &distribution_t : distribution) {
        // 遍历customer_site
        for (size_t i = 0; i < distribution_t.size(); i++) {
            fout << data.customer_site[i] << ":";
            for (size_t j = 0; j < distribution_t[i].size(); j++) {
                int edge_site = distribution_t[i][j].first;
                int stream_type = distribution_t[i][j].second;
                if (j == 0) {
                    fout << "<" + data.edge_site[edge_site] + "," + data.stream_type[stream_type] + ">";
                } else {
                    fout << ",<" + data.edge_site[edge_site] + "," + data.stream_type[stream_type] + ">";
                }
            }
            fout << "\n";
        }
    }
    fout.close();
}