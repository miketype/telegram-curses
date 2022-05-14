#include "td/actor/impl/ActorId-decl.h"
#include "td/tl/TlObject.h"
#include "td/utils/Status.h"
#include "windows/padwindow.h"
#include "td/generate/auto/td/telegram/td_api.h"
#include "td/generate/auto/td/telegram/td_api.hpp"
#include "telegram-curses.h"
#include "telegram-curses-window-root.h"
#include "chat-manager.h"
#include <memory>

namespace tdcurses {

class Tdcurses;

class DialogListWindow
    : public windows::PadWindow
    , public TdcursesWindowBase
    , public ChatManager {
 public:
  DialogListWindow(Tdcurses *root, td::ActorId<Tdcurses> root_actor) : TdcursesWindowBase(root, std::move(root_actor)) {
  }
  class Element;

  class Element
      : public windows::PadWindowElement
      , public Chat {
   public:
    Element(td::tl_object_ptr<td::td_api::chat> chat) : Chat(std::move(chat)) {
    }

    td::int32 render(TickitRenderBuffer *rb, bool is_selected) override;

    bool is_less(const windows::PadWindowElement &elx) const override {
      const Element &el = static_cast<const Element &>(elx);

      return order_ > el.order_ || (order_ == el.order_ && chat_id() > el.chat_id());
    }
    bool is_visible() const override {
      return order_ != 0;
    }

    void update_order() {
      order_ = get_order(td::td_api::chatListMain());
    }

    auto cur_order() const {
      return order_;
    }

   private:
    td::int64 order_{0};
  };

  void handle_input(TickitKeyEventInfo *info) override;

  void request_bottom_elements() override;
  void received_bottom_elements(td::Result<td::tl_object_ptr<td::td_api::ok>> R);

  template <typename T>
  void process_update(T &upd) {
    ChatManager::process_update(upd);
    auto chat = get_chat(upd.chat_id_);
    if (chat) {
      change_element(static_cast<Element *>(chat.get()));
    }
  }

  template <typename T>
  void process_user_update(T &upd) {
    ChatManager::process_update(upd);
  }

  void process_update(td::td_api::updateNewChat &update);
  void process_update(td::td_api::updateChatLastMessage &update);
  void process_update(td::td_api::updateChatPosition &update);

 private:
  bool running_req_{false};
  bool is_completed_{false};
};

}  // namespace tdcurses
