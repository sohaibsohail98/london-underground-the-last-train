#pragma once

#include "CoreMinimal.h"

/** Project-wide logging category. Use LT_LOG rather than raw UE_LOG. */
DECLARE_LOG_CATEGORY_EXTERN(LogLastTrain, Log, All);

#define LT_LOG(Verbosity, Format, ...) UE_LOG(LogLastTrain, Verbosity, Format, ##__VA_ARGS__)
