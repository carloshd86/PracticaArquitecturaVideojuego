#include "stdafx.h"
#include "globals.h"
#include "applicationmodemenu.h"
#include "applicationmanager.h"
#include "sys.h"
#include "core.h"
#include "font.h"
#include "asserts.h"
#include "memorycontrol.h"


ApplicationModeMenu::ApplicationModeMenu () :
	mCurrentContainer (nullptr),
	mMusicId          (0),
	m_pSprite         (nullptr) {}

// *************************************************
//
// *************************************************

ApplicationModeMenu::~ApplicationModeMenu()
{
	if (m_pSprite) g_pGraphicsEngine->ReleaseSprite(m_pSprite);

	auto it = mContainers.begin();
	while (it != mContainers.end()) 
	{
		GAME_DELETE((*it));
		it = mContainers.erase(it);
	}
}

// *************************************************
//
// *************************************************

IdMode ApplicationModeMenu::GetId()
{
	return AM_Menu;
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::Activate()
{
	GAME_ASSERT(g_pEventManager);

	m_pProperties = Properties::Instance("messages", g_pApplicationManager->GetLang());
	GAME_ASSERT(m_pProperties);

	g_pEventManager->Register(this, IEventManager::EM_Event::SinglePressUp     , 0);
	g_pEventManager->Register(this, IEventManager::EM_Event::SinglePressDown   , 0);
	g_pEventManager->Register(this, IEventManager::EM_Event::SinglePressEscape , 0);


	// Container main menu
	NavigationContainer * mainContainer = GAME_NEW(NavigationContainer, ());
	mCurrentContainer = mainContainer;

	Button * levelsButton  = InitButton(std::bind(&ApplicationModeMenu::OpenMenu, this, 1) , SCR_HEIGHT/4.f, 330.f, 200.f, SCR_HEIGHT*0.25f, mainContainer, "main_menu.levels.text");
	Button * optionsButton = InitButton(std::bind(&ApplicationModeMenu::OpenMenu, this, 2) , SCR_HEIGHT/4.f, 280.f, 200.f, SCR_HEIGHT*0.25f, mainContainer, "main_menu.options.text");
	Button * quitButton    = InitButton(std::bind(&ApplicationModeMenu::QuitGame, this)    , SCR_HEIGHT/4.f, 230.f, 200.f, SCR_HEIGHT*0.25f, mainContainer, "main_menu.exit.text", 1.f, 1.f, 0.f, 1.f, 0.7f, 0.f);

	mainContainer->FocusNextControl();
	mContainers.push_back(mainContainer);


	//Container select level menu
	NavigationContainer * levelContainer = GAME_NEW(NavigationContainer, ());

	Button * level1Button       = InitButton(std::bind(&ApplicationModeMenu::StartLevel , this, Game::GameLevel::Level1), SCR_HEIGHT/4.f, 330.f, 200.f, SCR_HEIGHT*0.25f, levelContainer, "main_menu.level1.text");
	Button * level2Button       = InitButton(std::bind(&ApplicationModeMenu::StartLevel , this, Game::GameLevel::Level2), SCR_HEIGHT/4.f, 280.f, 200.f, SCR_HEIGHT*0.25f, levelContainer, "main_menu.level2.text");
	Button * level3Button       = InitButton(std::bind(&ApplicationModeMenu::StartLevel , this, Game::GameLevel::Level3), SCR_HEIGHT/4.f, 230.f, 200.f, SCR_HEIGHT*0.25f, levelContainer, "main_menu.level3.text");
	Button * levelsReturnButton = InitButton(std::bind(&ApplicationModeMenu::OpenMenu    , this, 0)                     , SCR_HEIGHT/4.f, 180.f, 200.f, SCR_HEIGHT*0.25f, levelContainer, "main_menu.return.text", 1.f, 1.f, 0.f, 1.f, 0.7f, 0.f);

	levelContainer->SetVisible(false);
	mContainers.push_back(levelContainer);


	//Container options menu
	NavigationContainer * optionsContainer = GAME_NEW(NavigationContainer, ());
		     
	Button   * spanishButton       = InitButton   (std::bind(&ApplicationModeMenu::ChangeLanguage , this, Properties::P_Language::Spanish) , SCR_HEIGHT/4.f, 330.f, 200.f, SCR_HEIGHT*0.25f, optionsContainer, "main_menu.spanish.text");
	Button   * englishButton       = InitButton   (std::bind(&ApplicationModeMenu::ChangeLanguage , this, Properties::P_Language::English) , SCR_HEIGHT/4.f, 280.f, 200.f, SCR_HEIGHT*0.25f, optionsContainer, "main_menu.english.text");
	Checkbox * audioCheckbox       = InitCheckbox (                                                                                          SCR_HEIGHT/4.f, 230.f, 250.f, SCR_HEIGHT*0.25f, optionsContainer, "main_menu.activate_audio.text", g_pApplicationManager->IsAudioActivated());
	Slider   * audioSlider         = InitSlider   (                                                                                          SCR_HEIGHT/4.f, 180.f, 250.f, SCR_HEIGHT*0.25f, optionsContainer, "main_menu.volume.text"        );
	Button   * optionsReturnButton = InitButton   (std::bind(&ApplicationModeMenu::OpenMenu       , this, 0)                               , SCR_HEIGHT/4.f, 130.f, 200.f, SCR_HEIGHT*0.25f, optionsContainer, "main_menu.return.text", 1.f, 1.f, 0.f, 1.f, 0.7f, 0.f);

	optionsContainer->SetVisible(false);
	mContainers.push_back(optionsContainer);

	g_pGraphicsEngine->Init();
	m_pSprite = g_pGraphicsEngine->RequireSprite(vmake(SCR_HEIGHT/4.f, 400.f), vmake(400.f, 56.f), (DATA_FOLDER + "title.png").c_str(), false);

	mMusicId = g_pSoundManager->LoadWav((DATA_FOLDER + "Superboy.wav").c_str());
	ChangeActivatedAudio(audioCheckbox);
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::Deactivate()
{
	GAME_ASSERT(g_pEventManager);

	Properties::RemoveInstance();
	m_pProperties = nullptr;

	auto it = mContainers.begin();
	while (mContainers.end() != it)
	{
		const std::vector<Control *> containerControls = (*it)->GetControls();
		for (auto control : containerControls) g_pEventManager->Unregister(control);

		(*it)->Clear();
		it = mContainers.erase(it);
	}

	g_pEventManager->Unregister(this);
	g_pSoundManager->UnloadWav(mMusicId);
	g_pGraphicsEngine->End();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::ProcessInput()
{
	GAME_ASSERT(g_pEventManager);
	g_pEventManager->UpdateEvents();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::Run(float deltaTime)
{
	if (mCurrentContainer) mCurrentContainer->Update(deltaTime);
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::Render()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear( GL_COLOR_BUFFER_BIT );

	glColor3f(1.f, 1.f, 1.f);
	if (m_pSprite) g_pGraphicsEngine->RenderSprite(m_pSprite);
	mCurrentContainer->Render();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::ChangeLanguage(Properties::P_Language lang)
{
	Properties::P_Language applicationLanguage = g_pApplicationManager->GetLang();
	if (applicationLanguage != lang)
	{
		g_pApplicationManager->SetLang(lang);
		m_pProperties = Properties::Instance("messages", lang);
		GAME_ASSERT(m_pProperties);

		for (auto container : mContainers)
		{
			const std::vector<Control *> containerControls = container->GetControls();
			for (auto control : containerControls)
			{
				Button* button = dynamic_cast<Button*>(control);
				if (button) button->SetProperties(m_pProperties);
				else
				{
					Checkbox* checkbox = dynamic_cast<Checkbox*>(control);
					if (checkbox) checkbox->SetProperties(m_pProperties);
					else
					{
						Slider* slider = dynamic_cast<Slider*>(control);
						if (slider) slider->SetProperties(m_pProperties);
					}
				}
			}
		}
	}
}

// *************************************************
//
// *************************************************

bool ApplicationModeMenu::ProcessEvent(IEventManager::EM_Event event)
{
	switch (event) 
	{
		case IEventManager::EM_Event::SinglePressUp     : mCurrentContainer->FocusPreviousControl(); break;
		case IEventManager::EM_Event::SinglePressDown   : mCurrentContainer->FocusNextControl();     break;
		case IEventManager::EM_Event::SinglePressEscape : QuitGame();                                break;
	}

	return true;
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::OnClick(Button * button)
{
	mControlMap[button]();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::OnClick(Checkbox * checkbox)
{
	mControlMap[checkbox]();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::OnPercentageChanged(Slider* slider)
{
	mControlMap[slider]();
}

// *************************************************
//
// *************************************************

Button * ApplicationModeMenu::InitButton(std::function<void()> clickFunction, float x, float y, float width, float height, Container * parent, const char * textKey, float rOn, float gOn, float bOn, float rOff, float gOff, float bOff)
{
	Button * button = GAME_NEW(Button, (x, y, width, height, parent, m_pProperties, textKey, rOn, gOn, bOn, rOff, gOff, bOff));

	button->SetListener(this);

	g_pEventManager->Register(button, IEventManager::EM_Event::SinglePressEnter, 0);

	mControlMap[button] = clickFunction;

	return button;
}

// *************************************************
//
// *************************************************

Checkbox * ApplicationModeMenu::InitCheckbox(float x, float y, float width, float height, Container * parent, const char * textKey, bool checked, float rOn, float gOn, float bOn, float rOff, float gOff, float bOff)
{
	Checkbox * checkbox = GAME_NEW(Checkbox, (x, y, width, height, parent, m_pProperties, textKey, checked, rOn, gOn, bOn, rOff, gOff, bOff));

	checkbox->SetListener(this);

	g_pEventManager->Register(checkbox, IEventManager::EM_Event::SinglePressEnter, 0);

	mControlMap[checkbox] = std::bind(&ApplicationModeMenu::ChangeActivatedAudio, this, checkbox);

	return checkbox;
}

// *************************************************
//
// *************************************************

Slider * ApplicationModeMenu::InitSlider(float x, float y, float width, float height, Container * parent, const char * textKey, float rOn, float gOn, float bOn, float rOff, float gOff, float bOff)
{
	Slider * slider = GAME_NEW(Slider, (x, y, width, height, parent, m_pProperties, textKey, g_pApplicationManager->GetVolume(), rOn, gOn, bOn, rOff, gOff, bOff));

	slider->SetListener(this);

	g_pEventManager->Register(slider, IEventManager::EM_Event::MoveLeft    , 0);
	g_pEventManager->Register(slider, IEventManager::EM_Event::MoveRight   , 0);
	g_pEventManager->Register(slider, IEventManager::EM_Event::ReleaseLeft , 0);
	g_pEventManager->Register(slider, IEventManager::EM_Event::ReleaseRight, 0);

	mControlMap[slider] = std::bind(&ApplicationModeMenu::ChangeVolume, this, slider);

	return slider;
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::StartLevel(Game::GameLevel level)
{
	g_gameLevel = level;
	g_pApplicationManager->SwitchMode(AM_Game);
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::OpenMenu(int index)
{
	if (mCurrentContainer)
	{
		mCurrentContainer->SetVisible(false);
		Control * focusedControl = mCurrentContainer->GetFocusedControl();
		if (focusedControl) focusedControl->SetFocused(false);
	}

	if (static_cast<int>(mContainers.size()) > index) mCurrentContainer = mContainers[index];

	if (mCurrentContainer)
	{
		mCurrentContainer->SetVisible(true);
		mCurrentContainer->ResetFocus();
	}
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::ChangeActivatedAudio(Checkbox * checkbox)
{
	bool checkboxChecked = checkbox->IsChecked();
	g_pApplicationManager->SetAudioActivated(checkboxChecked);
	if (mMusicId) checkboxChecked ? g_pSoundManager->PlayMusic(mMusicId, g_pApplicationManager->GetVolume() / 100.f) : g_pSoundManager->StopMusic();
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::ChangeVolume(Slider* slider)
{
	float volume = slider->GetPercentage();
	if (mMusicId) g_pSoundManager->SetVolume(volume);
}

// *************************************************
//
// *************************************************

void ApplicationModeMenu::QuitGame()
{
	PostQuitMessage(0);
}