// MIT License

// Copyright (c) 2023 Zach Chan

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

// ------------------------------------------------------------------------------------------------
// Core
// ------------------------------------------------------------------------------------------------

// Controls whether or not asserts are enabled.
#define RF_ENABLE_ASSERTS true

// Determines the max number of audio commands that can be sent
// for each audio callback without incurring an allocation.
// Audio commands are send to the audio thread when
// "something happens". e.g.: a sound is played, or a volume is
// tweaked. If more than RF_MAX_AUDIO_COMMANDS, the underlying
// data structure holding the audio commands may reallocate
// during runtime.
#define RF_MAX_AUDIO_COMMANDS 1024

// Controls how many audio assets can be loaded at once.
#define RF_MAX_AUDIO_DATA 256

// Determines the array sized used for storing names for objects with names (cues, ...)
#define RF_MAX_NAME_SIZE 128

// The max amount of simultaneous sounds that RedFish can play.
#define RF_MAX_VOICES 256

// Enables SIMD for some operation. Enabling
// SIMD may increase performance.
// Set one of the values to 1 to enable a SIMD mode.
// Set all of the values to 0 to disable SIMD.
#define RF_USE_AVX 0
#define RF_USE_AVX_512 0
#define RF_USE_SSE 1

// ------------------------------------------------------------------------------------------------
// Mixing
// ------------------------------------------------------------------------------------------------

// Defines the max gain for some plug-ins.
#define RF_MAX_DECIBELS 12.0f

// Max amount of plug-ins that can be created on a mix group.
#define RF_MAX_MIX_GROUP_PLUGINS 5

// Max amount of sends that can be created on a mix group.
#define RF_MAX_MIX_GROUP_SENDS 5

// Max amount of mix groups that can be made, excluding the master mix group.
#define RF_MAX_MIX_GROUPS 64

// Defines silences, anything lower than RF_MIN_DECIBELS is silenced.
#define RF_MIN_DECIBELS -60.0f

// Controls the pan law for the Pan Plug-in. Set one of these values to 1.
#define RF_PAN_LAW_MINUS_FOUR_DOT_FIVE 0
#define RF_PAN_LAW_MINUS_SIX 0
#define RF_PAN_LAW_MINUS_THREE 1

// ------------------------------------------------------------------------------------------------
// Music
// ------------------------------------------------------------------------------------------------

// Controls the max number of layers a music cue can have.
#define RF_MAX_CUE_LAYERS 4

// Controls how many music cues can exist at once.
#define RF_MAX_CUES 64

// Controls how many stingers can exist at once.
#define RF_MAX_STINGERS 64

// Controls how many transitions can exist at once.
#define RF_MAX_TRANSITIONS 64