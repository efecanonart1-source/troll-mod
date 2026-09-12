#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// Troll Mode
//
// A collection of small, harmless "troll" effects that randomly trigger while
// playing Geometry Dash levels. Every effect can be toggled independently (or
// disabled entirely) from the mod settings page.
// ---------------------------------------------------------------------------

namespace {

    // Simple helper: rolls the dice against the user's configured "troll
    // chance" setting (stored as a percentage, 1-100).
    bool rollTroll() {
        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            return false;
        }
        int chance = Mod::get()->getSettingValue<int64_t>("troll-chance");
        int roll = rand() % 100 + 1;
        return roll <= chance;
    }

    bool settingOn(char const* key) {
        return Mod::get()->getSettingValue<bool>("enabled")
            && Mod::get()->getSettingValue<bool>(key);
    }

    std::vector<std::string> const& deathMessages() {
        static std::vector<std::string> messages = {
            "Skill issue.",
            "Was that... rage quit material?",
            "The wall said hello.",
            "Physics won this round.",
            "Have you tried not dying?",
            "That spike has seen a lot of players today.",
            "Achievement unlocked: Touched spike.",
            "Your controller is not broken. Probably.",
            "10/10 attempt, would watch again.",
            "The level editor is laughing right now.",
            "Practice mode exists, you know.",
            "Statistically, that was bound to happen.",
        };
        return messages;
    }

}

// ---------------------------------------------------------------------------
// PlayLayer hooks: screen wobble, fake percentage flashes, troll death popups
// ---------------------------------------------------------------------------

class $modify(TrollPlayLayer, PlayLayer) {

    struct Fields {
        float m_wobbleTimer = 0.f;
        bool m_isWobbling = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }
        // Fresh attempt, fresh chaos.
        m_fields->m_wobbleTimer = 0.f;
        m_fields->m_isWobbling = false;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            return;
        }
        if (!this->m_player1 || this->m_isPaused) {
            return;
        }

        // --- Screen wobble -------------------------------------------------
        if (settingOn("screen-wobble")) {
            if (!m_fields->m_isWobbling && rollTroll()) {
                // Small chance every frame-batch to start a brief wobble.
                m_fields->m_isWobbling = true;
                m_fields->m_wobbleTimer = 0.35f;
            }

            if (m_fields->m_isWobbling) {
                m_fields->m_wobbleTimer -= dt;

                if (auto gameLayer = this->m_objectLayer) {
                    float shakeX = static_cast<float>((rand() % 7) - 3);
                    float shakeY = static_cast<float>((rand() % 7) - 3);
                    gameLayer->setPositionX(gameLayer->getPositionX() + shakeX);
                    gameLayer->setPositionY(gameLayer->getPositionY() + shakeY);
                }

                if (m_fields->m_wobbleTimer <= 0.f) {
                    m_fields->m_isWobbling = false;
                }
            }
        }

        // --- Fake percentage flash ------------------------------------------
        if (settingOn("fake-percentage") && this->m_percentageLabel) {
            // Very low frequency flash so it's a surprise, not an annoyance.
            if ((rand() % 1000) < 3) {
                int fakeNumber = rand() % 100;
                auto realText = this->m_percentageLabel->getString();
                this->m_percentageLabel->setString(
                    (std::to_string(fakeNumber) + "%% (?!)").c_str()
                );

                // Restore the real text shortly after using an action.
                this->runAction(CCSequence::create(
                    CCDelayTime::create(0.25f),
                    CCCallLambda::create([this, realText = std::string(realText)]() {
                        if (this->m_percentageLabel) {
                            this->m_percentageLabel->setString(realText.c_str());
                        }
                    }),
                    nullptr
                ));
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        if (!settingOn("troll-death-messages")) {
            return;
        }
        // Don't troll every single death, that gets old fast.
        if (rand() % 100 >= 35) {
            return;
        }

        auto const& msgs = deathMessages();
        auto const& msg = msgs[rand() % msgs.size()];

        // Small delay so it doesn't collide with the game's own death UI.
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.6f),
            CCCallLambda::create([msg]() {
                FLAlertLayer::create("Troll Mode", msg, "Ok")->show();
            }),
            nullptr
        ));
    }
};

// ---------------------------------------------------------------------------
// PlayerObject hook: surprise jump inversion
// ---------------------------------------------------------------------------

class $modify(TrollPlayerObject, PlayerObject) {

    void pushButton(PlayerButton button) {
        if (settingOn("jump-inversion") && rollTroll()) {
            // Roughly a 1-in-N surprise: release instead of push, for one tap.
            // This is intentionally rare and cosmetic-scale mischief only.
            PlayerObject::releaseButton(button);
            return;
        }
        PlayerObject::pushButton(button);
    }
};

// ---------------------------------------------------------------------------
// Mod entrypoint
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    log::info("Troll Mode loaded. Chaos levels: nominal.");
}
