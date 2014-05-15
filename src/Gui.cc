/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Gui.h"
#include "Constants.h"
#include <gtk/gtk.h>
#include <algorithm>

const unsigned int MAX_PRESSED_KEYS = 6;
const unsigned int MAX_PRESSED_MODIFIERS = 8;

/**
 * Key information with name and pointer to widget.
 */
struct Key {
        Key () {}

        Key (std::initializer_list <std::string> l) {
                name = *l.begin ();
        }

        std::string name;
        GtkToggleButton *widget = nullptr;
};

/**
 * Vector of kays.
 */
typedef std::vector <Key> KeyVector;

/**
 * PIMPL for GUI.
 */
struct Gui::Impl {
        GtkWindow *window = nullptr;
        GtkBuilder *builder = nullptr;
        static void onClicked (GtkButton *button, gpointer user_data);
        void setButtonState (std::string const &name, bool state);
        void clearKeys ();
        void clearModifiers ();
        void clearMultimedia ();
        void setActiveManual (GtkToggleButton *togglebutton, bool on);

        // cat keyboard.ui | grep "0_" | grep -v "0_22[0-9]" | sed 's/<object class="GtkToggleButton" id="//g; s/">//g; s/ //g' | sort | awk -- '{ print "\"" $0 "\","; }'
        KeyVector KEYS = {
                        {"0_04"},
                        {"0_05"},
                        {"0_06"},
                        {"0_07"},
                        {"0_08"},
                        {"0_09"},
                        {"0_10"},
                        {"0_101"},
                        {"0_11"},
                        {"0_12"},
                        {"0_13"},
                        {"0_14"},
                        {"0_15"},
                        {"0_16"},
                        {"0_17"},
                        {"0_18"},
                        {"0_19"},
                        {"0_20"},
                        {"0_21"},
                        {"0_22"},
                        {"0_23"},
                        {"0_24"},
                        {"0_25"},
                        {"0_26"},
                        {"0_27"},
                        {"0_28"},
                        {"0_29"},
                        {"0_30"},
                        {"0_31"},
                        {"0_32"},
                        {"0_33"},
                        {"0_34"},
                        {"0_35"},
                        {"0_36"},
                        {"0_37"},
                        {"0_38"},
                        {"0_39"},
                        {"0_40"},
                        {"0_41"},
                        {"0_42"},
                        {"0_43"},
                        {"0_44"},
                        {"0_45"},
                        {"0_46"},
                        {"0_47"},
                        {"0_48"},
                        {"0_49"},
                        {"0_51"},
                        {"0_52"},
                        {"0_53"},
                        {"0_54"},
                        {"0_55"},
                        {"0_56"},
                        {"0_57"},
                        {"0_58"},
                        {"0_59"},
                        {"0_60"},
                        {"0_61"},
                        {"0_62"},
                        {"0_63"},
                        {"0_64"},
                        {"0_65"},
                        {"0_66"},
                        {"0_67"},
                        {"0_68"},
                        {"0_69"},
                        {"0_70"},
                        {"0_71"},
                        {"0_72"},
                        {"0_73"},
                        {"0_74"},
                        {"0_75"},
                        {"0_76"},
                        {"0_77"},
                        {"0_78"},
                        {"0_79"},
                        {"0_80"},
                        {"0_81"},
                        {"0_82"},
                        {"0_83"},
                        {"0_84"},
                        {"0_85"},
                        {"0_86"},
                        {"0_87"},
                        {"0_88"},
                        {"0_89"},
                        {"0_90"},
                        {"0_91"},
                        {"0_92"},
                        {"0_93"},
                        {"0_94"},
                        {"0_95"},
                        {"0_96"},
                        {"0_97"},
                        {"0_98"},
                        {"0_99"}
        };

        KeyVector MODIFIERS = {
                        {"0_224"}, // Left control
                        {"0_225"}, // Left shift
                        {"0_226"}, // Left alt
                        {"0_227"}, // Left GUI
                        {"0_228"}, // Right control
                        {"0_229"}, // Right shift
                        {"0_230"}, // Right alt
                        {"0_231"}  // Right GUI
        };

        KeyVector MULTIMEDIA = {
                        {"1_48"},  // Power
                        {"1_181"}, // Next track
                        {"1_182"}, // Previous track
                        {"1_183"}, // Stop
                        {"1_205"}, // Play / pause
                        {"1_226"}, // Mute
                        {"1_233"}, // Volume up
                        {"1_234"}  // Volume down
        };

        KeyVector activeKeys;
        KeyVector activeModifiers;
        Key const *activeMultimedia = nullptr;
};

/****************************************************************************/

Gui::Gui ()
{
        impl = new Impl;
}

/****************************************************************************/

Gui::~Gui ()
{
        delete impl;
}

/****************************************************************************/

void Gui::Impl::onClicked (GtkButton *button, gpointer userData)
{
        GtkToggleButton *togglebutton = GTK_TOGGLE_BUTTON (button);
        Impl *impl = static_cast <Impl *> (userData);
        std::string pressedName = gtk_buildable_get_name (GTK_BUILDABLE (togglebutton));
        bool pressedState = gtk_toggle_button_get_active (togglebutton);
        std::cerr << "Button : " << pressedName << " state : " << pressedState << std::endl;

        Key const *foundKey = nullptr;
        enum KeyType { TYPE_KEY, TYPE_MODIFIER, TYPE_MULTIMEDIA };
        KeyType foundType;

        auto check = [&pressedName] (Key const &key) {
                return key.name == pressedName;
        };

        KeyVector::const_iterator i;

        // Find what the toggled button is (KEY | MODIFIER | MULIMEDIA), and store it in a Key structure
        if ((i = std::find_if (impl->KEYS.begin (), impl->KEYS.end (), check)) != impl->KEYS.end ()) {
                foundKey = &*i;
                foundType = TYPE_KEY;
        }

        if ((i = std::find_if (impl->MODIFIERS.begin (), impl->MODIFIERS.end (), check)) != impl->MODIFIERS.end ()) {
                foundKey = &*i;
                foundType = TYPE_MODIFIER;
        }

        if ((i = std::find_if (impl->MULTIMEDIA.begin (), impl->MULTIMEDIA.end (), check)) != impl->MULTIMEDIA.end ()) {
                foundKey = &*i;
                foundType = TYPE_MULTIMEDIA;
        }


        // if KEY
        if (foundType == TYPE_KEY) {
                if (pressedState) {
                        // Find out how many keys are active (activeKeys.size ()).
                        // If 6 (MAX_PRESSED_KEYS) or more, do nothing.
                        if (impl->activeKeys.size () >= MAX_PRESSED_KEYS) {
                                impl->setActiveManual (togglebutton, 0);
                                return;
                        }

                        // Else : add this key to activeKeys and clearMultimedia
                        impl->activeKeys.push_back (*foundKey);
                        impl->clearMultimedia ();
                }
                else {
                        impl->activeKeys.erase (std::remove_if (impl->activeKeys.begin (), impl->activeKeys.end (), check));
                }
        }

        // if MODIFIER
        else if (foundType == TYPE_MODIFIER) {
                if (pressedState) {
                        // Find out how many modifiers are active (activeModifiers.size ()).
                        // If 8 (MAX_PRESSED_MODIFIERS) or more, do nothing. Redundand, because there are only 8 modifiers for now.
                        if (impl->activeModifiers.size () >= MAX_PRESSED_MODIFIERS) {
                                impl->setActiveManual (togglebutton, 0);
                                return;
                        }

                        // Else : add this key to activeModifiers and clearMultimedia
                        impl->activeModifiers.push_back (*foundKey);
                        impl->clearMultimedia ();
                }
                else {
                        impl->activeModifiers.erase (std::remove_if (impl->activeModifiers.begin (), impl->activeModifiers.end (), check));
                }
        }

        // if MULTIMEDIA
        else if (foundType == TYPE_MULTIMEDIA) {
                // clearMultimedia (all but our button), clearKeys, clearModifiers.
                if (pressedState) {
                        impl->clearKeys ();
                        impl->clearModifiers ();
                        impl->clearMultimedia ();
                        impl->setActiveManual (togglebutton, 1);
                        impl->activeMultimedia = foundKey;
                }
                else {
                        impl->activeMultimedia = nullptr;
                }
        }

        // Construct reportConfig to be sent to the device.
        // Send it.
        if (impl->activeMultimedia) {
                std::cerr << "Multimedia button : " << impl->activeMultimedia->name << std::endl;
        }
        else {
                std::cerr << "Modifiers : ";
                for (Key &key : impl->activeModifiers) {
                        std::cerr << key.name << " ";
                }

                std::cerr << "Keys : ";
                for (Key &key : impl->activeKeys) {
                        std::cerr << key.name << " ";
                }

                std::cerr << std::endl;
        }


}

void Gui::Impl::setButtonState (std::string const &name, bool state)
{

}

void Gui::Impl::clearKeys ()
{
        for (Key &key : activeKeys) {
                setActiveManual (key.widget, 0);
        }

        activeKeys.clear ();
}

void Gui::Impl::clearModifiers ()
{
        for (Key &key : activeModifiers) {
                setActiveManual (key.widget, 0);
        }

        activeModifiers.clear ();
}

void Gui::Impl::clearMultimedia ()
{
        if (activeMultimedia) {
                setActiveManual (activeMultimedia->widget, 0);
                activeMultimedia = nullptr;
        }
}

void Gui::Impl::setActiveManual (GtkToggleButton *togglebutton, bool on)
{
        g_signal_handlers_block_by_func (togglebutton, reinterpret_cast <void *> (&Gui::Impl::onClicked), this);
        gtk_toggle_button_set_active (togglebutton, on);
        g_signal_handlers_unblock_by_func (togglebutton, reinterpret_cast <void *> (&Gui::Impl::onClicked), this);
}

/****************************************************************************/

void Gui::init (int argc, char **argv)
{
//        auto &service = guard.service;
//        service.transmitConfiguration ({0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00});
//        service.receiveConfiguration ();

        gtk_init (&argc, &argv);

        impl->builder = gtk_builder_new_from_file ("keyboard.ui");

        for (Key &key : impl->KEYS) {
                GtkToggleButton *button = GTK_TOGGLE_BUTTON (gtk_builder_get_object(impl->builder, key.name.c_str ()));
                key.widget = button;
                int handlerId = g_signal_connect (button, "clicked", G_CALLBACK (&Gui::Impl::onClicked), impl);
                std::cerr << handlerId << std::endl;
        }

        for (Key &key : impl->MODIFIERS) {
                GtkToggleButton *button = GTK_TOGGLE_BUTTON (gtk_builder_get_object(impl->builder, key.name.c_str ()));
                key.widget = button;
                g_signal_connect (button, "clicked", G_CALLBACK (&Gui::Impl::onClicked), impl);
        }

        for (Key &key : impl->MULTIMEDIA) {
                GtkToggleButton *button = GTK_TOGGLE_BUTTON (gtk_builder_get_object(impl->builder, key.name.c_str ()));
                key.widget = button;
                g_signal_connect (button, "clicked", G_CALLBACK (&Gui::Impl::onClicked), impl);
        }

        /* Connect signal handlers to the constructed widgets. */
        impl->window = GTK_WINDOW (gtk_builder_get_object(impl->builder, "window"));
        g_signal_connect(impl->window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        gtk_widget_show_all (GTK_WIDGET (impl->window));
        gtk_main();
//        closeUsb();
}

