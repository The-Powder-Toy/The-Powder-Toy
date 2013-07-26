#pragma once

//Legacy blocking prompts
//This are not implemented here, but rather in the engine bootstrapper
bool ConfirmUI(std::string title, std::string message, std::string confirmText) {}

void ErrorUI(std::string title, std::string message) {}

void InformationUI(std::string title, std::string message) {}

std::string MessagePromptUI(std::string title, std::string message, std::string text, std::string placeholder) {}
