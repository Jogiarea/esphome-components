#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

/////////////////////// UartSensor ///////////////////////

bool UartSensor::handleValue(uint16_t value, TEntity::TVariant& current) {
    if (m_config.isSigned) {
        current = static_cast<int16_t>(value) / m_config.divider;
    } else {
        current = value / m_config.divider;
    }

    const float float_value = std::get<double>(current);
    const bool valid = !m_range.required() || (float_value >= m_range.min && float_value <= m_range.max);
    if (valid) {
        publish_state(float_value);
    } else {
        ESP_LOGE(TAG, "UartSensor::handleValue() => Sensor<%s> value<%f> out of range[%f, %f]",
            get_name().c_str(), float_value, m_range.min, m_range.max);
    }

    return valid;
}

/////////////////////// UartTextSensor ///////////////////////

bool UartTextSensor::handleValue(uint16_t value, TEntity::TVariant& current) {
    const auto it = m_map.findByKey(value);
    current = it != m_map.end() ? it->second : Utils::format("INVALID<%d>", value);
    publish_state(std::get<std::string>(current));
    return true;
}

/////////////////////// UartBinarySensor ///////////////////////

bool UartBinarySensor::handleValue(uint16_t value, TEntity::TVariant& current) {
    current = value > 0;
    publish_state(std::get<bool>(current));
    return true;
}



/////////////////////// Entities registrieren ///////////////////////

void register_entities(DaikinRotexUARTComponent *component) {
    // Flüssigkeitsleitung Temperatur
    auto t_liq = new UartSensor();
    t_liq->set_entity(TEntity::TEntityArguments(
        t_liq, "Temperatur Flüssigkeitsleitung",
        0x10, 0, false, 2, TEntity::Endian::Little,
        10.0, 1, {}, false
    ));
    component->add_entity(t_liq);

    // Rücklauftemperatur
    auto tr = new UartSensor();
    tr->set_entity(TEntity::TEntityArguments(
        tr, "Rücklauftemperatur",
        0x20, 0, false, 2, TEntity::Endian::Little,
        10.0, 1, {}, false
    ));
    component->add_entity(tr);

    // Vorlauftemperatur
    auto tv = new UartSensor();
    tv->set_entity(TEntity::TEntityArguments(
        tv, "Vorlauftemperatur",
        0x22, 0, false, 2, TEntity::Endian::Little,
        10.0, 1, {}, false
    ));
    component->add_entity(tv);

    // Außentemperatur
    auto outdoor_temp = new UartSensor();
    outdoor_temp->set_entity(TEntity::TEntityArguments(
        outdoor_temp, "Außentemperatur",
        0x30, 0, false, 2, TEntity::Endian::Little,
        10.0, 1, {}, false
    ));
    component->add_entity(outdoor_temp);

    // Betriebsart (Textsensor)
    auto mode = new UartTextSensor();
    mode->set_entity(TEntity::TEntityArguments(
        mode, "Betriebsart (UART)",
        0x21, 0, false, 1, TEntity::Endian::Little,
        1.0, 0, {}, false
    ));
    mode->set_map("0=Standby;1=Heizen;2=Kühlen;3=Warmwasser");
    component->add_entity(mode);

    // Heizstab Stufe 1
    auto buh1 = new UartBinarySensor();
    buh1->set_entity(TEntity::TEntityArguments(
        buh1, "Heizstab Stufe 1",
        0x60, 0, false, 1, TEntity::Endian::Little,
        1.0, 0, {}, false
    ));
    component->add_entity(buh1);

    // Heizstab Stufe 2
    auto buh2 = new UartBinarySensor();
    buh2->set_entity(TEntity::TEntityArguments(
        buh2, "Heizstab Stufe 2",
        0x61, 0, false, 1, TEntity::Endian::Little,
        1.0, 0, {}, false
    ));
    component->add_entity(buh2);

    // Abtauvorgang
    auto defrost = new UartBinarySensor();
    defrost->set_entity(TEntity::TEntityArguments(
        defrost, "Abtauvorgang",
        0x70, 0, false, 1, TEntity::Endian::Little,
        1.0, 0, {}, false
    ));
    component->add_entity(defrost);
}

} // namespace daikin_rotex_uart
} // namespace esphome
