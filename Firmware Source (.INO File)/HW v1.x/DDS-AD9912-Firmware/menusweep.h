#ifndef __MENU_SWEEP_H
#define __MENU_SWEEP_H

void DrawSweepMenu();
void SweepMenu();
void MakeSweep();
void TimerStart();
void TimerStop();
void TimerSetup();
void LoadSweepSettings();
void SaveSweepSettings();

#define SWEEP_SETTINGS_FLAG_ADR 102 // defualt settings

#endif