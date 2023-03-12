// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/Scroller.hpp"

using namespace ftxui;

// ============================
// ConsoleView
// ============================
class ConsoleView final
{
public:
	// Called whenever a command is successfully submitted
	using OnCommandSubmitFn = void( std::string_view command );
	using OnAutocompleteRequestFn = void( std::string_view command );
public:
	void Init( std::function<OnCommandSubmitFn> commandSubmit,
		std::function<OnAutocompleteRequestFn> autocompleteRequest );
	void Shutdown();

	void OnLog( const ConsoleMessage& message );
	bool OnUpdate( const float& deltaTime );

	void SetAutocompleteBuffer( const std::vector<std::string>& buffer );

private:
	// Handles CLI events i.e. input and scrolling
	bool ContainerEventHandler( Event e );
	void ConsumeCommand();
	void UpdateAutocomplete();

	bool IsInputValid() const;
	std::string GetCommandName() const;

	static const char* GenerateTimeString( const ConsoleMessage& message );
	static Element ConsoleMessageToFtxElement( const ConsoleMessage& message );

private:
	std::function<OnCommandSubmitFn> onCommandSubmit{ nullptr };
	std::function<OnAutocompleteRequestFn> onAutocompleteRequest{ nullptr };

	bool stopListening{ false };
	std::vector<ConsoleMessage> messages{};
	std::vector<std::string> autocompleteBuffer{};

	std::thread listenerThread;
	float timeToUpdate{ 0.1f };
	// If true, the network thread will have to wait for rendering to be finished
	bool isRenderingMessages{ false };
	// The user has entered a new command, jump to bottom to see the output
	bool jumpToBottom{ false };
	// The user has entered a new command, execute it on the main thread
	bool executeCommand{ false };

	// User input string
	std::string userInput{ "" };
	// Element that contains the autocomplete window
	// Is updated periodically instead of every frame
	Element autocompleteElement = text( "" );

	// The screen object where everything happens
	ScreenInteractive screen = ScreenInteractive::Fullscreen();
	// Frame of the little animation in the top-right corner
	int animationFrame{ 0 };
	// Title bar on the top
	Component consoleTitleComponent{};
	// Text input bar on the bottom
	Component inputFieldComponent{};
	// Displays the actual ConsoleMessages
	Component messageFrameComponent{};
	// Parent of messageFrameComponent
	Component messageScrollerComponent{};
	// Logical container for inputFieldComponent and messageScrollerComponent
	Component containerComponent{};
	// Parent of all the above
	Component mainComponent{};
};
