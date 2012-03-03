/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "smoothmovement.hpp"

#include <misc/log.hpp>

namespace fluo {
namespace world {

SmoothMovement::SmoothMovement(boost::shared_ptr<world::ServerObject> obj, CL_Vec3f targetLoc, unsigned int durationMillis) :
        movingObject_(obj), targetLoc_(targetLoc), duration_(durationMillis), wasStarted_(false) {
}

bool SmoothMovement::wasStarted() const {
    return wasStarted_;
}

bool SmoothMovement::isFinished() const {
    return totalElapsedMillis_ >= duration_;
}

void SmoothMovement::start() {
    wasStarted_ = true;
    totalElapsedMillis_ = 0;
    startLoc_ = movingObject_->getLocation();
    diff_ = targetLoc_ - startLoc_;
}

void SmoothMovement::finish(bool interrupted) {
    // necessary to set this to the precise target location. otherwise rounding errors might add up
    if (!interrupted) {
        movingObject_->setLocation(targetLoc_);
    }
    
    if (finishedCallback_) {
        finishedCallback_();
    }
}

void SmoothMovement::update(unsigned int elapsedMillis) {
    if (totalElapsedMillis_ + elapsedMillis >= duration_) {
        elapsedMillis = duration_ - totalElapsedMillis_;
    }
    
    CL_Vec3f movementDelta = diff_ * ((float)elapsedMillis / duration_);

    CL_Vec3f curLoc = movingObject_->getLocation() + movementDelta;
    movingObject_->setLocation(curLoc);

    totalElapsedMillis_ += elapsedMillis;
}

void SmoothMovement::setFinishedCallback(FinishedCallback cb) {
    finishedCallback_ = cb;
}

}
}
