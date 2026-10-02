#include "flow.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

IMPORT_SERVICE(FlowService, svc_flow);
IMPORT_SERVICE(MessageService, svc_message);

static int applicationStartTime;

constexpr uint16_t kMessageGroup = 0;   // Midna flow BMG group
constexpr uint16_t kMidnaSpeaker = 21;  // Midna speaker value

// Vanilla flow IDs
constexpr uint16_t kMidnaPromptHumanNode = 0x018c;
constexpr uint16_t kMidnaPromptWolfNode = 0x018d;
constexpr uint16_t kMidnaHumanBranch = 0x0190;
constexpr uint16_t kMidnaWolfBranch = 0x0193;
constexpr uint16_t kMidnaTalkNode = 0x018f;
constexpr uint16_t kMidnaHumanTalkEdge = 0x0113;
constexpr uint16_t kMidnaWolfTalkEdge = 0x0119;

constexpr uint16_t kMidnaMenuPromptEntry = 3003;    // Message index for flow message data

// Languages
constexpr std::array kAllLanguages{
    MESSAGE_LANGUAGE_ENGLISH,
    MESSAGE_LANGUAGE_GERMAN,
    MESSAGE_LANGUAGE_FRENCH,
    MESSAGE_LANGUAGE_SPANISH,
    MESSAGE_LANGUAGE_ITALIAN,
    MESSAGE_LANGUAGE_JAPANESE,
};

// Styles
constexpr mods::flow::MessageStyle kResponseStyle =
    mods::flow::MessageStyle{}.speaker(kMidnaSpeaker).box_kind(MESSAGE_BOX_MIDNA);
constexpr mods::flow::MessageStyle kPromptStyle =
    kResponseStyle.draw_type(MESSAGE_DRAW_INSTANT).talk_anim(31).face_anim(31);

// Globals
FlowHandler* g_flowSelf;
mods::flow::Graph g_graph;
std::vector<mods::flow::RegisteredMessage> g_messages;
std::vector<mods::flow::MessageOverride> g_overrides;
mods::flow::Event g_applicationEvent;
mods::flow::Event g_openShopEvent;
std::string_view g_shopOptMessage = "You haven't started an\napplication! Please apply!";
std::string_view g_applicationMessage = "You haven't started an\napplication! Let's get yours started!";

static void applicationEventFunc(ModContext*, const FlowEventContext*, void*) {
    if (!g_flowSelf->getConfigHandler()->getApplicationStatus()) {
        g_flowSelf->getConfigHandler()->setProcessApplication(true);
        g_flowSelf->getConfigHandler()->setCvarApplicationStatus(STATUS_ONGOING);
        g_flowSelf->getConfigHandler()->setUpdateFlowFlag(true);
    }
}

static void openShopEventFunc(ModContext*, const FlowEventContext* event, void* user_data) {
    auto* self = static_cast<FlowHandler*>(user_data);
    g_flowSelf->getUiHandler()->openCarcoShop();    // Test this
}

mods::flow::RegisteredMessage register_message(const mods::flow::MessageBuilder& builder) {
    std::vector<mods::flow::MessageVariant> variants;
    variants.reserve(kAllLanguages.size());
    for (const MessageLanguage language : kAllLanguages) {
        variants.push_back(builder.build(language));
    }
    return mods::flow::register_message(kMessageGroup, variants);
}

ModResult add_message(const mods::flow::MessageBuilder& builder, MessageId& outId) {
    auto message = register_message(builder);
    if (!message) {
        return message.result();
    }
    outId = message.id();
    g_messages.push_back(std::move(message));
    return MOD_OK;
}

inline ModResult build_selection(uint8_t speaker, std::string_view opt_1, std::string_view opt_2,
                       std::string_view opt_3, MessageId& id) {
    return add_message(mods::flow::MessageBuilder{}.speaker(speaker)
                       .options(opt_1, opt_2, opt_3), id);
}

mods::flow::MessageBuilder build_prompt(std::string_view prefix, std::string_view post_player_name_text,
                                        std::string_view suffix) {
    return mods::flow::MessageBuilder{kPromptStyle}
        .text(prefix)
        .text_color(MESSAGE_COLOR_RED)
        .player_name()
        .text_color(MESSAGE_COLOR_DEFAULT)
        .text("?\n")
        .text_scale(85)
        .text(suffix)
        .text_scale(100)
        .await_choice();
}

inline ModResult build_message(uint8_t speaker, std::string_view message, MessageId& id) {
        return add_message(mods::flow::MessageBuilder{kPromptStyle}.text(message), id);
}

ModResult FlowHandler::initialize(ConfigHandler* config_handler, UiHandler* ui_handler, ModError* error) {
    g_flowSelf = this;
    configHandler = config_handler;
    uiHandler = ui_handler;
    updateStatus = STATUS_NOT_UPDATED;
    configHandler->setUpdateFlowFlag(false);

    g_applicationEvent = mods::flow::register_event("start_application_timer activation", applicationEventFunc, this);
    g_openShopEvent = mods::flow::register_event("open_shop_event", openShopEventFunc, this);
    ModResult result = buildMidnaPromptFlow();
    if (result != MOD_OK) {
        return mods::set_error(error, result, "failed to register custom messages");
    }
    return result;
}

ModResult FlowHandler::buildMidnaPromptFlow() {
    MessageId humanSelectionId = 0;
    MessageId wolfSelectionId = 0;
    MessageId carcoCorpPromptId = 0;
    MessageId carcoCorpSelectionId = 0;
    MessageId carcoCorpShopMessageId = 0;
    MessageId shopOpenedMessageId = 0;
    MessageId applicationMessageId = 0;

    ModResult result = add_message(mods::flow::MessageBuilder{}
                                   .speaker(kMidnaSpeaker).options("Transform into human", "Warp", "CarcoCorp"),
                                   humanSelectionId);
    if (result == MOD_OK) {
        result = add_message(mods::flow::MessageBuilder{}
                             .speaker(kMidnaSpeaker).options("Transform into wolf", "Warp", "CarcoCorp"),
                             wolfSelectionId);  
    }
    if (result == MOD_OK) {
        result = add_message(build_prompt("I love CarcoCorp, ", "!\n", "Choose an option..."), carcoCorpPromptId);
    }
    if (result == MOD_OK) {
        result = build_selection(kMidnaSpeaker, "Shop", "Application", "Talk", carcoCorpSelectionId);
    }
    if (result == MOD_OK) {
        result = build_message(kMidnaSpeaker, g_shopOptMessage, carcoCorpShopMessageId);
    }
    if (result == MOD_OK) {
        result = build_message(kMidnaSpeaker, "Thank you for partnering with CarcoCorp!", shopOpenedMessageId);
    }
    if (result == MOD_OK) {
        result = build_message(kMidnaSpeaker, g_applicationMessage, applicationMessageId);
    }

    if (result != MOD_OK) {
        return result;
    }

    mods::flow::GraphBuilder graph{kMessageGroup};
    const auto carcoCorpSetup = graph.add_event(FLOW_EVENT_SELECT_VERTICAL, {0, 0, 0, 4});
    const auto shopOpenedMessage = graph.add_message(shopOpenedMessageId).next(carcoCorpSetup);
    const auto shopEvent = graph.add_event(g_openShopEvent.id(), {0, 0, 0, 0}).next(shopOpenedMessage);
    const auto shopMessage = graph.add_message(carcoCorpShopMessageId);
    switch (configHandler->getApplicationStatus()) {
        case STATUS_NOT_STARTED:
        case STATUS_ONGOING:
            shopMessage.next(carcoCorpSetup);
            break;
        case STATUS_FINISHED:
            shopMessage.next(shopEvent);
            break;
    }
    const auto applicationMessage = graph.add_message(applicationMessageId).next(kMidnaTalkNode);
    const auto applicationEvent = graph.add_event(g_applicationEvent.id(), {0, 0, 0, 0}).next(applicationMessage);
    const auto carcoCorpBranch = graph.add_branch(FLOW_QUERY_SELECT_3_CANCEL, 0).results({shopMessage, applicationEvent, kMidnaTalkNode, mods::flow::kEnd});
    const auto carcoCorpSelection = graph.add_message(carcoCorpSelectionId).next(carcoCorpBranch);
    const auto carcoCorpPrompt = graph.add_message(carcoCorpPromptId).next(carcoCorpSelection);
    carcoCorpSetup.next(carcoCorpPrompt);

    const auto humanSelection = graph.add_message(humanSelectionId).next(kMidnaHumanBranch);
    const auto wolfSelection = graph.add_message(wolfSelectionId).next(kMidnaWolfBranch);
    graph.patch_node(kMidnaPromptHumanNode, mods::flow::message(0, kMidnaMenuPromptEntry, humanSelection));
    graph.patch_node(kMidnaPromptWolfNode, mods::flow::message(0, kMidnaMenuPromptEntry, wolfSelection));
    graph.patch_edge(kMidnaHumanTalkEdge, carcoCorpSetup);
    graph.patch_edge(kMidnaWolfTalkEdge, carcoCorpSetup);

    g_graph = graph.commit();
    if (!g_graph) {
        return MOD_ERROR;
    }
    
    return result;
}

void FlowHandler::updateFlow() {
    if (configHandler->getUpdateFlowFlag()) {
        switch (configHandler->getApplicationStatus()) {
            case STATUS_NOT_STARTED:
                g_shopOptMessage = "You haven't started an\napplication! Please apply!";
                g_applicationMessage = "You haven't started an\napplication! Let's get yours started!";
                break;

            case STATUS_ONGOING:
                g_shopOptMessage = "Your application is still processing!";
                g_applicationMessage = "We are working on your application!\nPlease check back later!";
                break;

            case STATUS_FINISHED:
                g_shopOptMessage = "Opening the CarcoCorp Store!";
                g_applicationMessage = "Your application has already\nbeen approved!";
                break;
        }

        buildMidnaPromptFlow();
        configHandler->setUpdateFlowFlag(false);
    }
}

void FlowHandler::shutdown() {
    g_graph.reset();
    g_messages.clear();
}