// Fill out your copyright notice in the Description page of Project Settings.


#include "TankPlayerState.h"

int32 ATankPlayerState::GetPlayerId() {
	return PlayerId;
}

void ATankPlayerState::SetScore(float score) {
	Score = score;
}
void ATankPlayerState::GiveScore(float score) {
	Score += score;
}
float ATankPlayerState::GetScore() {
	return Score;
}