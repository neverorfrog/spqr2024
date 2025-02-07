/**
 * @file InterceptorTwoCard.cpp
 *
 * This file implements a basic behavior for Interceptor.
 *
 * @author Flavio Volpi
 */

#include "Representations/BehaviorControl/FieldBall.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/MotionControl/WalkingEngineOutput.h"
#include "Representations/Configuration/BallSpecification.h"
#include "Representations/Modeling/BallModel.h"

#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/BHMath.h"
#include "Tools/Math/Geometry.h"
#include <Eigen/Geometry>
#include <iostream>

using Line2 = Eigen::Hyperplane<float,2>;

CARD(InterceptorTwoCard,
{,
  CALLS(Activity),
  CALLS(LookAtBall),
  CALLS(WalkToPoint),
  CALLS(Say),
  CALLS(RecordTargetAndSpeed),
  CALLS(WalkAtRelativeSpeed),
  CALLS(KeyframeMotion),
  REQUIRES(FieldBall),
  REQUIRES(BallModel),
  REQUIRES(FieldDimensions),
  REQUIRES(RobotPose),
  REQUIRES(RobotInfo),
  REQUIRES(WalkingEngineOutput),
  REQUIRES(BallSpecification),
  DEFINES_PARAMETERS(
  {,
    (float)(0.8f) walkSpeed,
    (int)(100) initialWaitTime,
    (int)(7000) ballNotSeenTimeout,
    (float)(0.4f) toleranceTime,
  }),
});

class InterceptorTwoCard : public InterceptorTwoCardBase
{


    
  bool preconditions() const override
  {
    /*  INTERCEPTOR 2
     *  Interceptor that find the intersection point between the 
     *  line startPosition-EndPosition of the ball and its perpendicular
     *  passing through the position of the robot.
    **/

    bool condition = getCondition();
    return condition;
    
  }

  bool postconditions() const override
  {
    /* INTERCEPTOR 2*/
    bool condition = getCondition();
    return !condition;
  }

  option
  {
    theActivitySkill(BehaviorStatus::Dummy);

    initial_state(start)
    {
      transition
      {
        
      }

      action
      {
        theSaySkill("two");
        theLookAtBallSkill();
        
        Vector2f direction = theBallModel.estimate.velocity / theBallModel.estimate.velocity.norm();
        Vector2f intersection = Geometry::getOrthogonalProjectionOfPointOnLine(theFieldBall.recentBallPositionRelative(), direction, Vector2f(0.f, 0.f));
        // theWalkToPointSkill(intersection);

        // TO TEST: 
        // 1) si potrebbe mettere theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, +-1.f))
        // 2) motionRequest

        // TESTED:
        // theRecordTargetAndSpeedSkill(intersection, 0.f);
        // theWalkAtRelativeSpeedSkill(Pose2f(0.f, velocity_speed.x(), velocity_speed.y()));
        
        // if(intersection.y() < 0)
        //   theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, -1.f));
        // else 
        //   theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, 1.f));
        
        // theKeyframeMotionSkill(KeyframeMotionRequest::genuflectStand);


        // Vector2f velocity_speed = intersection / intersection.norm();
        if(intersection.y() < 0)
          theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, -1.f));
        else 
          theWalkAtRelativeSpeedSkill(Pose2f(0.f, 0.f, 1.f));
        

      }
    } 
  }

  bool getCondition() const
  {
    if(theBallModel.estimate.velocity.norm() != 0){
      Vector2f direction = theBallModel.estimate.velocity / theBallModel.estimate.velocity.norm();
      Vector2f intersection = Geometry::getOrthogonalProjectionOfPointOnLine(theFieldBall.recentBallPositionRelative(), direction, Vector2f(0.f, 0.f));
      if(Geometry::isPointInsideRectangle(theFieldBall.recentBallPositionRelative(), theFieldBall.recentBallEndPositionRelative(), intersection)){
        
        const float distanceBallToIntersection = (theFieldBall.recentBallPositionRelative() - intersection).norm();
        const float distanceRobotToIntersection = intersection.norm();
        
        float timeBallToIntersection = BallPhysics::timeForDistance(theBallModel.estimate.velocity, distanceBallToIntersection, theBallSpecification.friction); // in seconds
        float timeRobotToIntersection = abs(distanceRobotToIntersection / theWalkingEngineOutput.maxSpeed.translation.x());                                     // in seconds
          
        bool condition = (timeBallToIntersection != std::numeric_limits<float>::max()) && (timeRobotToIntersection <= timeBallToIntersection + toleranceTime);

        return condition;
      }
    }
    return false;

  }
};

MAKE_CARD(InterceptorTwoCard);
