/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: neha
 *
 * Created on July 2, 2017, 3:15 PM
 */


//#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <boost/filesystem.hpp>

//using namespace std;

/*
 * 
 */
namespace fs = ::boost::filesystem;

// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
void get_all(std::string dataPath, const std::string& ext, std::vector<fs::path>& ret)
{
    const fs::path root = dataPath.c_str();
    if(!fs::exists(root) || !fs::is_directory(root)) return;

    fs::recursive_directory_iterator it(root);
    fs::recursive_directory_iterator endit;

    while(it != endit)
    {
        if(fs::is_regular_file(*it) && it->path().extension() == ext) 
        {
            std::stringstream depthFile;
            depthFile << dataPath << it->path().filename().string();
            ret.push_back(depthFile.str());
        }
        ++it;

    }

}

bool isFileExist(char const* filename)
{
    std::ifstream file_tmp(filename);
    if (!file_tmp.is_open())
    {
        return false;
    }
    file_tmp.close();
    return true;
}

double convertexr2png(std::string dataPath)
{
    
    std::stringstream depthFilename;
   // depthFilename << dataPath << "human_" << pose << "_depth_" << angle <<"_0001.exr";
    
    std::cout<<"Got Filenames\n";
    depthFilename << dataPath;
    
    bool isExist = isFileExist(depthFilename.str().c_str());
    if (isExist == false){
        std::cout<<"The file "<<depthFilename.str()<<" does not exist\n";
        return false;
    }
    cv::Mat depthImage;
    std::cout<<"depth111...\n";
    depthImage = cv::imread(depthFilename.str(), -1);//cv::IMREAD_ANYDEPTH); 
    std::cout<<"depth...\n";
    
    cv::Mat ch1, ch2, ch3;
    // "channels" is a vector of 3 Mat arrays:
    std::vector<cv::Mat> channels(3);
    // split img:
    cv::split(depthImage, channels);
    // get the channels (dont forget they follow BGR order in OpenCV)
    ch1 = channels[0];
    ch2 = channels[1];
    ch3 = channels[2];
    
    cv::Mat depthImage_short(depthImage.size(), CV_16U);
    for (int y = 0; y < depthImage.rows; y++)
    {
        for (int x = 0; x < depthImage.cols; x++)
        {
            const float d = depthImage.at<cv::Vec3f>(y, x)[0];
        
            if (d < 100.0f)
                depthImage_short.at<unsigned short>(y, x) = d * 1000;
            else 
                depthImage_short.at<unsigned short>(y, x) = 100000;
            
        }
    }
    
    
    std::cout<<"Got Depth\n";
    
    std::string delimiter = ".";
    std::string newFilename = dataPath.substr(0, dataPath.find(delimiter));
    
    std::stringstream depthFilename1;
    depthFilename1 << newFilename <<".png";
    
    //std::stringstream depthFilename1;
    //depthFilename1 << dataPath << "human_" << pose << "_depth_" << angle <<".png";
    cv::imwrite(depthFilename1.str(), depthImage_short);
    
    depthImage = depthImage_short;
    
    std::cout<<"Wrote Depth\n";
    
    //Check Range
    double minVal; 
    double maxVal; 
    cv::minMaxLoc( depthImage, &minVal, &maxVal);
    
    
    std::cout << "depthImage.dims = " << depthImage.dims << " depthImage.size = " << depthImage.size() << " depthImage.channels = " << depthImage.channels() << std::endl;
    
    std::cout << "min val depth: " << minVal << "\n";
    std::cout << "max val depth: " << maxVal << "\n";
    
    
    return minVal;
}

int main(int argc, char** argv) {
    std::cout<<"Hello, dearest world\n";
    std::string dataPath = "/home/neha/Documents/TUM_Books/projects/IDP/segmentation/segmentation_python/raw_data/";
   
    //
    //bool convert = convertexr2png(depthFilename.str(),0,0);
    
    std::string ext = ".exr";
    std::vector<fs::path> paths;
    
    get_all(dataPath,ext,paths);
    
    double min = 100000;
    for(int i=0; i < paths.size(); i++)
    {
        //std::cout<<"path is : "<<paths[i]<<"\n";
        double minVal = convertexr2png(paths[i].c_str());
        if(min > minVal)
        {
            min = minVal;
        }
    }
    std::cout<<"min is : "<<min<<"\n";
    for(int i=0; i < paths.size(); i++)
    {
        if( remove( paths[i].c_str() ) != 0 )
            perror( "Error deleting file" );
        else
            puts( "File successfully deleted" );
    }
    
    return 0;
}
