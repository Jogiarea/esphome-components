#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

#include <algorithm>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
: m_message_manager()
, m_later_calls()
, m_project_git_hash_sensor(nullptr)
, m_project_git_hash()
{
}



void DaikinRotexUARTComponent::add_entity(EntityBase* pEntityBase) {
    if (auto pEntity = dynamic_cast<daikin_rotex_uart::TEntity*>(pEntityBase)) {
        m_message_manager.add(pEntity);
    }
}



void DaikinRotexUARTComponent::setup() {
    m_project_git_hash_sensor->publish_state(m_project_git_hash);
}

void DaikinRotexUARTComponent::loop() {
    // Sende die nächste Anfrage an die Daikin-Einheit
    m_message_manager.sendNextRequest(*this);

    // Eingehende Bytes prüfen und verarbeiten
    while (this->available() > 0) {
        uint8_t byte;
        if (this->read_byte(&byte)) {
            // Übergib das Byte an den MessageManager zur Dekodierung
            m_message_manager.handleResponse(*this);
        }
    }

    // Später geplante Aufrufe ausführen
    for (auto it = m_later_calls.begin(); it != m_later_calls.end();) {
        if (millis() > it->second) {
            it->first();
            it = m_later_calls.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace daikin_rotex_uart
} // namespace esphome
