#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <chrono>

#include "library/music_library.h"
#include "main_component.h"
#include "gui/gui_resources.h"

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
            setUsingNativeTitleBar (true);
            setContentOwned (new chow_tunes::Main_Component, true);

            setResizable (true, false);
            centreWithSize (getWidth(), getHeight());

            juce::Component::setVisible (true);

            const auto fs = cmrc::gui::get_filesystem();
            const auto icon_file = fs.open ("icon.png");
            const auto icon = juce::ImageCache::getFromMemory (icon_file.begin(), (int) icon_file.size());

            setIcon (icon);
            getPeer()->setIcon (icon);
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
        {
            quit();
            return;
        }

        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void suspended() override
    {
        dynamic_cast<chow_tunes::Main_Component*> (mainWindow->getContentComponent())->audio_player.reset();
    }

    void resumed() override
    {
        auto* main = dynamic_cast<chow_tunes::Main_Component*> (mainWindow->getContentComponent());
        main->audio_player.emplace();
        main->app_state.volume_db.changeBroadcaster();
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

    void systemHotkeyPressed (uint64_t key_id) override
    {
        dynamic_cast<chow_tunes::Main_Component*> (mainWindow->getContentComponent())->hotkey_handler.handle_hotkey_callback (key_id);
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

                namespace chrono = std::chrono;
                const auto start = chrono::high_resolution_clock::now();
                auto music_library = chow_tunes::library::index_directory (args[(int) idx + 1].toStdString().c_str());
                const auto duration = chrono::high_resolution_clock::now() - start;

                //                std::printf ("%s\n", chow_tunes::library::print_library (music_library).c_str());
                std::printf ("Scanned %d songs, from %d albums, from %d artists, in %d milliseconds\n",
                             (int) music_library.songs.size(),
                             (int) music_library.albums.size(),
                             (int) music_library.artists.size(),
                             (int) chrono::duration_cast<chrono::milliseconds> (duration).count());

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
