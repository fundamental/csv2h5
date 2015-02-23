/*
 * Copyright (c) 2015 Mark McCurry
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <H5Cpp.h>
//TODO Find the total of a CSV file to estimate the time to convert it to hdf5
#define LINE_INPUT_MAX 8*1024

using std::vector;
using std::string;

const char *help = "Usage: csv2h5 INPUT.csv OUTPUT.h5\n\n"
"csv2h5 - Convert csv file consisting of floating point data\n"
"         and header into a HDF5 file";

void check_args(int argc, char **argv)
{
    if(argc != 3) {
        puts(help);
        exit(1);
    }
    const char *csv = argv[1];
    const char *h5  = argv[2];
    if(!strstr(csv,"csv")) {
        printf("ERROR: '%s' doesn't look like a csv file\n", csv);
        printf("       Please use the .csv extension\n");
        exit(1);
    }
    
    if(!strstr(h5,"h5")) {
        printf("ERROR: '%s' doesn't look like a h5 file\n", csv);
        printf("       Please use the .h5 extension\n");
        exit(1);
    }
}

vector<string> extract_names(const char *header)
{
    vector<string> names;
    std::stringstream ss(header);

    string res;
    while(ss.good())
    {
        char c;
        ss.read(&c,1);
        if(c == ',') {
            names.push_back(res);
            res = "";
        } else
            res = res+c;
    }
    return names;
}

int main(int argc, char **argv)
{
    check_args(argc, argv);
    char line[LINE_MAX];//8Kbi lines
    const char *fin  = argv[1];
    const char *fout = argv[2];

    FILE *file = fopen(fin, "r");

    //Get the names of the columns
    fgets(line,LINE_INPUT_MAX,file);
    vector<string> names = extract_names(line);
    vector<vector<float> > data;

    for(unsigned i=0; i<names.size(); ++i)
        data.push_back(vector<float>());

    printf("Loading Data From CSV...\n");
    while(fgets(line, LINE_INPUT_MAX, file))
    {
        float f;
        int ii=0;
        std::stringstream sss(line);
        while (sss >> f)
        {
            data[ii].push_back(f);
            ii++;

            if (sss.peek() == ',')
                sss.ignore();
        }
    }

    for(unsigned i=0; i<data.size(); ++i)
        printf("col[%d] => %d sample(s)\n", i, (int)data[i].size());

    printf("Writing to HDF5...\n");
    try
    {
        H5::H5File hFile(fout, H5F_ACC_TRUNC);
        for(unsigned i=0; i<data.size(); ++i) {
            hsize_t dims[1];
            dims[0] = data[i].size();
            H5::DataSpace dataspace(1,dims);
            H5::IntType datatype(H5::PredType::NATIVE_FLOAT);
            H5::DataSet dataset = 
                hFile.createDataSet(names[i], datatype, dataspace);
            dataset.write(data[i].data(), H5::PredType::NATIVE_FLOAT);
        }
    } catch(H5::Exception e)
    {
        return 1;
    }
    return 0;
}
