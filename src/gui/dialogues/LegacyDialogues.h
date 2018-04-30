#pragma once

//Legacy blocking prompts
//This are not implemented here, but rather in the engine bootstrapper
bool ConfirmUI(String title, String message, String confirmText) {}

void ErrorUI(String title, String message) {}

void InformationUI(String title, String message) {}

String MessagePromptUI(String title, String message, String text, String placeholder) {}
