#include <chowdsp_data_structures/chowdsp_data_structures.h>

#include "main_component.h"
#include "library/music_library.h"

class ChowTunesApplication : public juce::JUCEApplication
{
public:
    ChowTunesApplication() = default;

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& window_name)
            : DocumentWindow (window_name,
                              juce::Desktop::getInstance()
                                  .getDefaultLookAndFeel()
                                  .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
#if JUCE_IOS || JUCE_ANDROID
            setContentOwned (new MainComponent ("Bridge"), true);
            setFullScreen (true);
#else
            setUsingNativeTitleBar (true);
            setContentOwned (new chow_tunes::Main_Component, true);

            setResizable (true, false);
            centreWithSize (getWidth(), getHeight());
#endif
            juce::Component::setVisible (true);

            //            Image icon = ImageCache::getFromMemory (BinaryData::logo_256_png, BinaryData::logo_256_pngSize);
            //            setIcon (icon);
            //            getPeer()->setIcon (icon);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    const juce::String getApplicationName() override { return "ChowTunes"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; } // TODO
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& commandLine) override
    {
        if (handleInternalCommandLineOperations (commandLine))
            quit();

        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..
        mainWindow = nullptr; // (deletes our window)
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        if (handleInternalCommandLineOperations (commandLine))
            return;
    }

    bool handleInternalCommandLineOperations ([[maybe_unused]] const juce::String& commandLine) // NOLINT
    {
        juce::StringArray args;
        args.addTokens (commandLine, " ", "");

        for (const auto [idx, arg] : chowdsp::enumerate (args))
        {
            if (arg == "--index")
            {
                if (args.size() < (int) idx + 2)
                {
                    std::printf ("The index parameter must be used with an additional directory argument!\n");
                    return true;
                }

                chow_tunes::library::index_directory (args[(int) idx + 1].toStdString().c_str());

                return true;
            }
        }

        return false;
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (ChowTunesApplication)
