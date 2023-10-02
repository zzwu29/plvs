/*
 * This file is part of PLVS.
 * This file is a modified version present in RGBDSLAM2 (https://github.com/felixendres/rgbdslam_v2)
 * Copyright (C) 2018-present Luigi Freda <luigifreda at gmail dot com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2021 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ORBMATCHER_H
#define ORBMATCHER_H

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include"sophus/sim3.hpp"

#include "MapPoint.h"
#include "KeyFrame.h"
#include "Frame.h"
#include "Pointers.h"


namespace PLVS2
{

    class ORBmatcher
    {
    public:

        ORBmatcher(float nnratio=0.6, bool checkOri=true);

        // Computes the Hamming distance between two ORB descriptors
        static int DescriptorDistance(const cv::Mat &a, const cv::Mat &b);

        // Search matches between Frame keypoints and projected MapPoints. Returns number of matches
        // Used to track the local map (Tracking)
        int SearchByProjection(Frame &F, const std::vector<MapPointPtr> &vpMapPoints, const float th=3, const bool bFarPoints = false, const float thFarPoints = 50.0f);

        // Project MapPoints tracked in last frame into the current frame and search matches.
        // Used to track from previous frame (Tracking)
        int SearchByProjection(Frame &CurrentFrame, const Frame &LastFrame, const float th, const bool bMono);

        // Project MapPoints seen in KeyFrame into the Frame and search matches.
        // Used in relocalisation (Tracking)
        int SearchByProjection(Frame &CurrentFrame, KeyFramePtr& pKF, const std::set<MapPointPtr> &sAlreadyFound, const float th, const int ORBdist);

        // Project MapPoints using a Similarity Transformation and search matches.
        // Used in loop detection (Loop Closing)
        int SearchByProjection(KeyFramePtr& pKF, Sophus::Sim3<float> &Scw, const std::vector<MapPointPtr> &vpPoints, std::vector<MapPointPtr> &vpMatched, int th, float ratioHamming=1.0);

        // Project MapPoints using a Similarity Transformation and search matches.
        // Used in Place Recognition (Loop Closing and Merging)
        int SearchByProjection(KeyFramePtr& pKF, Sophus::Sim3<float> &Scw, const std::vector<MapPointPtr> &vpPoints, const std::vector<KeyFramePtr> &vpPointsKFs, std::vector<MapPointPtr> &vpMatched, std::vector<KeyFramePtr> &vpMatchedKF, int th, float ratioHamming=1.0);

        // Search matches between MapPoints in a KeyFrame and ORB in a Frame.
        // Brute force constrained to ORB that belong to the same vocabulary node (at a certain level)
        // Used in Relocalisation and Loop Detection
        int SearchByBoW(KeyFramePtr& pKF, Frame &F, std::vector<MapPointPtr> &vpMapPointMatches);
        int SearchByBoW(KeyFramePtr& pKF1, KeyFramePtr& pKF2, std::vector<MapPointPtr> &vpMatches12);

        // Matching for the Map Initialization (only used in the monocular case)
        int SearchForInitialization(Frame &F1, Frame &F2, std::vector<cv::Point2f> &vbPrevMatched, std::vector<int> &vnMatches12, int windowSize=10);

        // Matching to triangulate new MapPoints. Check Epipolar Constraint.
        int SearchForTriangulation(KeyFramePtr& pKF1, KeyFramePtr& pKF2,
                                   std::vector<pair<size_t, size_t> > &vMatchedPairs, const bool bOnlyStereo, const bool bCoarse = false);

        // Search matches between MapPoints seen in KF1 and KF2 transforming by a Sim3 [s12*R12|t12]
        // In the stereo and RGB-D case, s12=1
        // int SearchBySim3(KeyFramePtr& pKF1, KeyFramePtr& pKF2, std::vector<MapPointPtr> &vpMatches12, const float &s12, const cv::Mat &R12, const cv::Mat &t12, const float th);
        int SearchBySim3(KeyFramePtr& pKF1, KeyFramePtr& pKF2, std::vector<MapPointPtr> &vpMatches12, const Sophus::Sim3f &S12, const float th);

        // Project MapPoints into KeyFrame and search for duplicated MapPoints.
        int Fuse(KeyFramePtr& pKF, const vector<MapPointPtr> &vpMapPoints, const float th=3.0, const bool bRight = false);

        // Project MapPoints into KeyFrame using a given Sim3 and search for duplicated MapPoints.
        int Fuse(KeyFramePtr& pKF, Sophus::Sim3f &Scw, const std::vector<MapPointPtr> &vpPoints, float th, vector<MapPointPtr> &vpReplacePoint);

    public:

        static const int TH_LOW;
        static const int TH_HIGH;
        static const int HISTO_LENGTH;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    protected:

        bool CheckDistEpipolarLine(const cv::KeyPoint &kp1, const cv::KeyPoint &kp2, const cv::Mat &F12, const KeyFramePtr& pKF, const bool b1=false);
        bool CheckDistEpipolarLine2(const cv::KeyPoint &kp1, const cv::KeyPoint &kp2, const cv::Mat &F12, const KeyFramePtr& pKF, const float unc);

        float RadiusByViewingCos(const float &viewCos);

        void ComputeThreeMaxima(std::vector<int>* histo, const int L, int &ind1, int &ind2, int &ind3);

        float mfNNratio;
        bool mbCheckOrientation;
    };

}// namespace PLVS2

#endif // ORBMATCHER_H
