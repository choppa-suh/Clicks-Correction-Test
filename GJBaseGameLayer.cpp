#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <array>
#include <cstdint>

#include "../ClickCorrection.hpp"

using namespace geode::prelude;

namespace {
    constexpr int JUMP = static_cast<int>(PlayerButton::Jump);
}

/*
 * Click correction (early-click forgiveness)
 * ------------------------------------------
 * In Geometry Dash a click made while a cube/ball/robot/spider is still in
 * the air is simply lost if the button is released before landing. This hook
 * remembers such a click for up to `max-frames` physics steps. If the player
 * touches the ground inside that window and the button is no longer held, the
 * click is replayed for one step so the jump / gravity flip still happens.
 *
 * If the button is still held when the player lands, the game's own hold
 * handling already takes care of it, so the buffered click is just dropped.
 */
class $modify(CCGameLayer, GJBaseGameLayer) {
    struct Slot {
        bool held = false;           // is the real button currently down?
        bool pending = false;        // do we have an early click buffered?
        uint64_t expiresAtFrame = 0; // last frame the buffered click is valid
        double expiresAtTime = 0.0;  // safety net based on level time
        bool releaseQueued = false;  // we injected a press and owe a release
        uint64_t releaseAtFrame = 0;
    };

    struct Fields {
        uint64_t frame = 0;
        bool injecting = false;
        std::array<Slot, 2> slots{};
    };

    // Decide whether a click right now would be wasted (and so is worth buffering).
    bool shouldBuffer(PlayerObject* player) {
        if (!player || player->m_isDead) return false;
        if (player->m_isOnGround) return false;  // the click works normally
        if (player->m_touchedRing) return false; // may be consumed by an orb
        if (player->m_isDashing) return false;
        // These modes react to clicks in mid-air, nothing to correct.
        if (player->m_isShip || player->m_isBird || player->m_isDart || player->m_isSwing) {
            return false;
        }
        return true;
    }

    PlayerObject* playerFor(bool isPlayer1) {
        return isPlayer1 ? m_player1 : m_player2;
    }

    void injectJump(bool down, bool isPlayer1) {
        auto f = m_fields.self();
        f->injecting = true;
        GJBaseGameLayer::handleButton(down, JUMP, isPlayer1);
        f->injecting = false;
    }

    void clearCorrections() {
        auto f = m_fields.self();
        for (auto& slot : f->slots) {
            slot.pending = false;
            slot.releaseQueued = false;
        }
    }

    // Runs once per physics step, before the game processes its own commands.
    void serviceCorrections() {
        auto f = m_fields.self();
        double now = m_gameState.m_levelTime;

        for (int i = 0; i < 2; i++) {
            bool isPlayer1 = (i == 0);
            auto& slot = f->slots[i];

            // 1) Finish a correction we started on an earlier step.
            if (slot.releaseQueued && f->frame >= slot.releaseAtFrame) {
                slot.releaseQueued = false;
                if (!slot.held) this->injectJump(false, isPlayer1);
            }

            if (!slot.pending) continue;

            auto player = this->playerFor(isPlayer1);
            bool expired = f->frame > slot.expiresAtFrame || now > slot.expiresAtTime;
            if (!player || player->m_isDead || expired) {
                slot.pending = false;
                continue;
            }

            if (player->m_isOnGround) {
                slot.pending = false;
                // Still holding: the game already jumps on landing by itself.
                if (slot.held) continue;

                this->injectJump(true, isPlayer1);
                slot.releaseQueued = true;
                slot.releaseAtFrame = f->frame + 1;
            }
        }
    }

    // The game funnels every jump press / release through here.
    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();

        if (f->injecting || button != JUMP) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto& slot = f->slots[isPlayer1 ? 0 : 1];
        slot.held = down;

        if (down) {
            // A fresh physical press cancels any correction we still owed.
            slot.releaseQueued = false;

            if (clickcorrection::isEnabled() && this->shouldBuffer(this->playerFor(isPlayer1))) {
                int frames = clickcorrection::maxFrames();
                slot.pending = true;
                slot.expiresAtFrame = f->frame + static_cast<uint64_t>(frames);
                slot.expiresAtTime = m_gameState.m_levelTime
                    + (static_cast<double>(frames) * 2.0 + 4.0) / clickcorrection::DEFAULT_TPS;
            }
            else {
                slot.pending = false;
            }
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto f = m_fields.self();

        if (!isHalfTick) {
            f->frame++;
            this->serviceCorrections();
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }

    void resetPlayer() {
        this->clearCorrections();
        auto f = m_fields.self();
        for (auto& slot : f->slots) slot.held = false;
        GJBaseGameLayer::resetPlayer();
    }
};
