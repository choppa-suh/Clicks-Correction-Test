#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include "../ClickCorrection.hpp"
#include "../ui/CorrectionPopup.hpp"

using namespace geode::prelude;

class $modify(CCPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (!clickcorrection::showPauseButton()) return;

        auto winSize = CCDirector::get()->getWinSize();

        // Own menu, so we do not depend on any other mod's node IDs.
        auto menu = CCMenu::create();
        menu->setID("click-correction-menu"_spr);
        menu->setPosition({0.f, 0.f});
        menu->setContentSize(winSize);

        auto label = CCLabelBMFont::create("CC", "bigFont.fnt");
        auto sprite = CircleButtonSprite::create(
            label, CircleBaseColor::Green, CircleBaseSize::Small
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(CCPauseLayer::onClickCorrection)
        );
        button->setID("click-correction-button"_spr);
        button->setPosition({winSize.width - 28.f, winSize.height - 28.f});

        menu->addChild(button);
        this->addChild(menu, 10);
    }

    void onClickCorrection(CCObject*) {
        CorrectionPopup::create()->show();
    }
};
