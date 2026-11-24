#pragma once

#include "GrenadePredictor/GrenadePredictorState.h"
#include "GrenadeTimer/GrenadeTimerState.h"
#include "ModelGlow/ModelGlowState.h"
#include "PlayerInfoInWorld/PlayerInfoInWorldState.h"

struct VisualFeaturesStates {
    PlayerInfoInWorldState playerInfoInWorldState;
    ModelGlowState modelGlowState;
    GrenadeTimerState grenadeTimerState;
    GrenadePredictorState grenadePredictorState;
};
