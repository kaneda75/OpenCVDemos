#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#define RESULT_CSV_FILE        "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/results/result.csv"
#define DETECTORS_TYPE_FILE    "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/files/detectors.txt"
#define DESCRIPTORS_TYPE_FILE  "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/files/descriptors.txt"
#define MATCHERS_TYPE_FILE 	   "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/files/matchers.txt"

using namespace cv;
using namespace std;

const string queryImageName = "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/images/Sabata_1995_Antoni_T_pies creu.jpg";
const string dirToSaveResImages = "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/results";



const string fileWithTrainImages = "//Users/xescriche/Documents/MASTER LINUX/PFM/PACS/PAC3.2 (31 octubre)/probes/images/trainImagesAnother.txt";
//const string colorString = "CV_LOAD_IMAGE_UNCHANGED";
const string colorString = "CV_LOAD_IMAGE_UNCHANGED";
const string nomFotoAComparar = "VanGogh-starry_night_ballance1.jpg";

const int NUM_DETECTORS = 26; 	// NUM_DETECTORS - 1 because postion 0 of array
const int NUM_DESCRIPTORS = 7; 	// NUM_DESCRIPTORS -1
const int NUM_MATCHERS = 4; 	// NUM_MATCHERS -1

static void maskMatchesByTrainImgIdx(const vector<DMatch>& matches, int trainImgIdx, vector<char>& mask) {
        mask.resize(matches.size());
        fill(mask.begin(), mask.end(), 0);
        for (size_t i = 0; i < matches.size(); i++) {
            if (matches[i].imgIdx == trainImgIdx)
                mask[i] = 1;
        }
}

static void readTrainFilenames(const string& filename, string& dirName, vector<string>& trainFilenames) {
        trainFilenames.clear();
        ifstream file(filename.c_str());
        if (!file.is_open())
            return;

        size_t pos = filename.rfind('\\');
        char dlmtr = '\\';
        if (pos == String::npos) {
            pos = filename.rfind('/');
            dlmtr = '/';
        }
        dirName = pos == string::npos ? "" : filename.substr(0, pos) + dlmtr;

        while (!file.eof()) {
            string str;
            getline(file, str);
            if (str.empty()) break;
            trainFilenames.push_back(str);
        }
        file.close();
}

static bool createDetectorDescriptorMatcher(const string& detectorType, const string& descriptorType, const string& matcherType,Ptr<FeatureDetector>& featureDetector,Ptr<DescriptorExtractor>& descriptorExtractor,Ptr<DescriptorMatcher>& descriptorMatcher)  {
        featureDetector = FeatureDetector::create(detectorType);
        descriptorExtractor = DescriptorExtractor::create(descriptorType);
        descriptorMatcher = DescriptorMatcher::create(matcherType);

    bool isCreated = !(featureDetector.empty() || descriptorExtractor.empty() || descriptorMatcher.empty());
    if (!isCreated)
        cout << "No es pot crear el detector, el descriptor o el matcher." << endl << ">" << endl;
    return isCreated;
}

static bool readImages(const string& queryImageName, const string& trainFilename,Mat& queryImage, vector <Mat>& trainImages, vector<string>& trainImageNames, int color, int & numTrainImages) {
        queryImage = imread(queryImageName, color);
        if (queryImage.empty()) {
            cout << "La imatge consulta no pot ser llegida." << endl << ">" << endl;
            return false;
        }
        string trainDirName;
        readTrainFilenames(trainFilename, trainDirName, trainImageNames);
        if (trainImageNames.empty()) {
            cout << "Les imatges a comparar no poden ser llegides." << endl << ">" << endl;
            return false;
        }
        int readImageCount = 0;
        for (size_t i = 0; i < trainImageNames.size(); i++) {
            string filename = trainDirName + trainImageNames[i];
            Mat img = imread(filename, color);
            if (img.empty())
                cout << "La imatge a comparar " << filename << " no pot ser llegida." << endl;
            else
                readImageCount++;
            trainImages.push_back(img);
        }
        if (!readImageCount) {
            cout << "Totes les imatges a comparar no poden ser llegides." << endl << ">" << endl;
            return false;
        } else {
            numTrainImages = readImageCount;
            cout << "Number of train images:                        " << readImageCount << endl;
        }

    return true;
}

static void detectKeypoints(const Mat& queryImage, vector<KeyPoint>& queryKeypoints,const vector<Mat>& trainImages, vector<vector<KeyPoint> >& trainKeypoints,Ptr<FeatureDetector>& featureDetector) {
        featureDetector->detect(queryImage, queryKeypoints);
        featureDetector->detect(trainImages, trainKeypoints);

}

static void computeDescriptors(const Mat& queryImage, vector<KeyPoint>& queryKeypoints, Mat& queryDescriptors,const vector<Mat>& trainImages, vector<vector<KeyPoint> >& trainKeypoints, vector<Mat>& trainDescriptors,Ptr<DescriptorExtractor>& descriptorExtractor, int & numQueryDescriptors, int & numTrainDescriptors) {
        descriptorExtractor->compute(queryImage, queryKeypoints, queryDescriptors);
        descriptorExtractor->compute(trainImages, trainKeypoints, trainDescriptors);
        int totalTrainDesc = 0;
        for (vector<Mat>::const_iterator tdIter = trainDescriptors.begin(); tdIter != trainDescriptors.end(); tdIter++)
            totalTrainDesc += tdIter->rows;
        
        numQueryDescriptors = queryDescriptors.rows;
        cout << "Number of query image descriptors:     " << numQueryDescriptors << endl;
        numTrainDescriptors = totalTrainDesc;
        cout << "Number of train images descriptors:     " << totalTrainDesc << endl;
}

static void matchDescriptors(const Mat& queryDescriptors, const vector<Mat>& trainDescriptors, vector<DMatch>& matches, Ptr<DescriptorMatcher>& descriptorMatcher, int & numMatches, double & matchTime) {
        TickMeter tm;
        tm.start();
        descriptorMatcher->add(trainDescriptors);
        descriptorMatcher->train();
        tm.stop();
        tm.start();
        descriptorMatcher->match(queryDescriptors, matches);
        tm.stop();
        matchTime = tm.getTimeMilli();
        CV_Assert(queryDescriptors.rows == (int) matches.size() || matches.empty());
        numMatches = matches.size();
        cout << "Number of matches:                              " << numMatches << endl;
        cout << "Match time:                                            " << matchTime << " ms" << endl;
}

static void saveResultImages(const Mat& queryImage, const vector<KeyPoint>& queryKeypoints,const vector<Mat>& trainImages, const vector<vector<KeyPoint> >& trainKeypoints, const vector<DMatch>& matches, const vector<string>& trainImagesNames, const string& resultDir, string nomArxiu) {
        Mat drawImg;
        vector<char> mask;
        for (size_t i = 0; i < trainImages.size(); i++) {
            if (!trainImages[i].empty()) {
                maskMatchesByTrainImgIdx(matches, (int) i, mask);
                drawMatches(queryImage, queryKeypoints, trainImages[i], trainKeypoints[i],
                        matches, drawImg, Scalar(255, 0, 0), Scalar(0, 255, 255), mask);
                string filename = resultDir + "/" + nomArxiu + "_" + trainImagesNames[i];
                if (!imwrite(filename, drawImg))
                    cout << "L'imatge " << filename << " no pot ser guardada (pot ser que el directori " << resultDir << " no existeixi)." << endl;
            }
        }
}

int leerLineaTxt(FILE *ftxt, char * linea) {
	int error = 0;
	char c = ' ';

	do {
		if (fgets(linea, 120, ftxt) == NULL) {
			error = 1; // ERROR_LECTURA;
		} else {
			c = linea[0];
		};
	} while ((!feof(ftxt)) && (!error)
			&& ((c == '.') || (c == ' ') || (c == '\r') || (c == '\n')
					|| (c == '\0')));
	return (error);
} /* f_leerLinea */

void readFileDetectors(string detectors[NUM_DETECTORS]) {
	FILE* ftxt;
	char linea[120] = "\0";
	int error2 = 0;
	int i = 0;
	char file[120];
	sprintf(file, DETECTORS_TYPE_FILE);
	ftxt = fopen(file, "rt");
	if (ftxt != NULL) {
		error2 = leerLineaTxt(ftxt, linea);
		while ((!feof(ftxt)) && (!error2)) {
			detectors[i] = linea;
			detectors[i].erase(detectors[i].length() - 1, 1);
			i++;
			if (!error2)
				leerLineaTxt(ftxt, linea);
		}
		fclose(ftxt);
	}
}

void readFileDetescriptors(string descriptors[NUM_DESCRIPTORS]) {
	FILE* ftxt;
	char linea[120] = "\0";
	int error2 = 0;
	int i = 0;
	char file[120];
	sprintf(file, DESCRIPTORS_TYPE_FILE);
	ftxt = fopen(file, "rt");
	if (ftxt != NULL) {
		error2 = leerLineaTxt(ftxt, linea);
		while ((!feof(ftxt)) && (!error2)) {
			descriptors[i] = linea;
			descriptors[i].erase(descriptors[i].length() - 1, 1);
			i++;
			if (!error2)
				leerLineaTxt(ftxt, linea);
		}
		fclose(ftxt);
	}
}

void readFileMatchers(string matchers[NUM_MATCHERS]) {
	FILE* ftxt;
	char linea[120] = "\0";
	int error2 = 0;
	int i = 0;
	char file[120];
	sprintf(file, MATCHERS_TYPE_FILE);
	ftxt = fopen(file, "rt");
	if (ftxt != NULL) {
		error2 = leerLineaTxt(ftxt, linea);
		while ((!feof(ftxt)) && (!error2)) {
			matchers[i] = linea;
			matchers[i].erase(matchers[i].length() - 1, 1);
			i++;
			if (!error2)
				leerLineaTxt(ftxt, linea);
		}
		fclose(ftxt);
	}
}

void computeMatching() {
	string detectorType;
	string descriptorType;
	string matcherType;

	bool fer = true;

	const int numExcepcions = 2;
	string excepcions[numExcepcions][2];

	excepcions[0][0] = "ORB";
	excepcions[0][1] = "SIFT";

	excepcions[1][0] = "ORB";
	excepcions[1][1] = "OpponentSIFT";

	int color = -1;
	int id = 0;

	string detectors[NUM_DETECTORS];
	readFileDetectors(detectors);

	string descriptors[NUM_DESCRIPTORS];
	readFileDetescriptors(descriptors);

	string matchers[NUM_MATCHERS];
	readFileMatchers(matchers);

	for (int i = 0; i < NUM_DETECTORS; ++i) {
		detectorType = detectors[i];
		for (int j = 0; j < NUM_DESCRIPTORS; ++j) {
			descriptorType = descriptors[j];
			for (int k = 0; k < NUM_MATCHERS; ++k) {
				matcherType = matchers[k];

				int numTrainImages = 0;
				int numQueryDescriptors = 0;
				int numTrainDescriptors = 0;
				int numMatches = 0;
				double matchTime = 0;
				fer = true;

				id++;

				for (int k = 0; k < numExcepcions; ++k) {
					if (detectorType == excepcions[k][0] &&
						descriptorType == excepcions[k][1]) {
						fer = false;
						goto control;
					}
				}

	control:
				if (fer) {
					try {
						Ptr<FeatureDetector> featureDetector;
						Ptr<DescriptorExtractor> descriptorExtractor;
						Ptr<DescriptorMatcher> descriptorMatcher;

						cout << endl;
						cout << "Detector:                                               " << detectorType << endl;
						cout << "Descriptor:                                             " << descriptorType << endl;
						cout << "Matcher:                                                " << matcherType << endl;

						if (!createDetectorDescriptorMatcher(detectorType,descriptorType, matcherType, featureDetector,descriptorExtractor, descriptorMatcher)) {
							cout << endl;
						}

						Mat queryImage;
						vector<Mat> trainImages;
						vector<string> trainImagesNames;
						if (!readImages(queryImageName, fileWithTrainImages,queryImage, trainImages, trainImagesNames,color, numTrainImages)) {
							cout << endl;
						}

						vector<KeyPoint> queryKeypoints;
						vector<vector<KeyPoint> > trainKeypoints;
						detectKeypoints(queryImage, queryKeypoints, trainImages,trainKeypoints, featureDetector);

						Mat queryDescriptors;
						vector<Mat> trainDescriptors;
						computeDescriptors(queryImage, queryKeypoints,queryDescriptors, trainImages, trainKeypoints,trainDescriptors, descriptorExtractor,numQueryDescriptors, numTrainDescriptors);

						vector<DMatch> matches;
						matchDescriptors(queryDescriptors, trainDescriptors, matches, descriptorMatcher, numMatches,matchTime);

						saveResultImages(queryImage, queryKeypoints,trainImages, trainKeypoints, matches,trainImagesNames, dirToSaveResImages,detectorType + "_" + descriptorType + "_" + matcherType);

						FILE *ftxt;
						char resultFile[30];
						sprintf(resultFile, RESULT_CSV_FILE);
						ftxt = fopen(resultFile, "a");
						if (ftxt != NULL) {
							fprintf(ftxt, "%d;", id);
							fprintf(ftxt, "%s;", nomFotoAComparar.c_str());
							fprintf(ftxt, "%s;", detectorType.c_str());
							fprintf(ftxt, "%s;", descriptorType.c_str());
							fprintf(ftxt, "%s;", matcherType.c_str());
							fprintf(ftxt, "%s;", colorString.c_str());
//							fprintf(ftxt, "%d;", numTrainImages);
							fprintf(ftxt, "%d;", numQueryDescriptors);
							fprintf(ftxt, "%d;", numTrainDescriptors);
							fprintf(ftxt, "%d;", numMatches);
							fprintf(ftxt, "%f;", matchTime);
							fprintf(ftxt, "%s\n", "");
							fclose(ftxt);
						}
					} catch (exception& e) {
						cout << e.what() << endl;
						FILE *ftxt;
						char resultFile[30];
						sprintf(resultFile, RESULT_CSV_FILE);
						ftxt = fopen(resultFile, "a");
						if (ftxt != NULL) {
							fprintf(ftxt, "%d;", id);
							fprintf(ftxt, "%s;", nomFotoAComparar.c_str());
							fprintf(ftxt, "%s;", detectorType.c_str());
							fprintf(ftxt, "%s;", descriptorType.c_str());
							fprintf(ftxt, "%s;", matcherType.c_str());
							fprintf(ftxt, "%s;", colorString.c_str());
	//							fprintf(ftxt, "%d;", numTrainImages);
							fprintf(ftxt, "%d;", numQueryDescriptors);
							fprintf(ftxt, "%d;", numTrainDescriptors);
							fprintf(ftxt, "%d;", numMatches);
							fprintf(ftxt, "%f;", matchTime);
							fprintf(ftxt, "%s", e.what());
							fclose(ftxt);
						}
					}
				} else {
					FILE *ftxt;
					char resultFile[30];
					sprintf(resultFile, RESULT_CSV_FILE);
					ftxt = fopen(resultFile, "a");
					if (ftxt != NULL) {
						fprintf(ftxt, "%d;", id);
						fprintf(ftxt, "%s;", nomFotoAComparar.c_str());
						fprintf(ftxt, "%s;", detectorType.c_str());
						fprintf(ftxt, "%s;", descriptorType.c_str());
						fprintf(ftxt, "%s;", matcherType.c_str());
						fprintf(ftxt, "%s;", colorString.c_str());
//							fprintf(ftxt, "%d;", numTrainImages);
						fprintf(ftxt, "%d;", numQueryDescriptors);
						fprintf(ftxt, "%d;", numTrainDescriptors);
						fprintf(ftxt, "%d;", numMatches);
						fprintf(ftxt, "%f;", matchTime);
						fprintf(ftxt, "%s\n","Signal : EXC_BAD_ACCESS:Could not access memory");
						fclose(ftxt);
					}
				}

			}
		}
	}
}

int main(int argc, char *argv[]) {
	computeMatching();
}
