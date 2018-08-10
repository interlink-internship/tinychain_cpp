//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_PARAMS_H
#define TINYCHAIN_CPP_PARAMS_H

// The infamous max block size.
const int MAX_BLOCK_SERIALIZED_SIZE = 1000000;

// Coinbase transaction outputs can be spent after this many blocks have
// elapsed since being mined.
//
// This is "100" in bitcoin core.
const int COINBASE_MATURITY = 2;

// Accept blocks timestamped as being from the future, up to this amount.
const int MAX_FUTURE_BLOCK_TIME = (60 * 60 * 2);

// The number of Belushis per coin. #realname COIN
const int BELUSHIS_PER_COIN = 100e6;

const int TOTAL_COINS = 21000000;

// The maximum number of Belushis that will ever be found.
const int MAX_MONEY = BELUSHIS_PER_COIN * TOTAL_COINS;

// The duration we want to pass between blocks being found, in seconds.
// This is lower than Bitcoin's configuation (10 * 60).
//
// #realname PowTargetSpacing
const int TIME_BETWEEN_BLOCKS_IN_SECS_TARGET = 1 * 60;

// The number of seconds we want a difficulty period to last.
//
// Note that this differs considerably from the behavior in Bitcoin, which
// is configured to target difficulty periods of (10 * 2016) minutes.
//
// #realname PowTargetTimespan
const int DIFFICULTY_PERIOD_IN_SECS_TARGET = (60 * 60 * 10);

//After this number of blocks are found, adjust difficulty.
//
// #realname DifficultyAdjustmentInterval
const double DIFFICULTY_PERIOD_IN_BLOCKS = (DIFFICULTY_PERIOD_IN_SECS_TARGET / TIME_BETWEEN_BLOCKS_IN_SECS_TARGET);

// The number of right-shifts applied to 2 ** 256 in order to create the
// initial difficulty target necessary for mining a block.
const int INITIAL_DIFFICULTY_BITS = 24;

// The number of blocks after which the mining subsidy will halve.
//
// #realname SubsidyHalvingInterval
const int HALVE_SUBSIDY_AFTER_BLOCKS_NUM = 210000;

#endif //TINYCHAIN_CPP_PARAMS_H
