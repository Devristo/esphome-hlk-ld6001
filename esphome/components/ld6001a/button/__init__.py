import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    CONF_FACTORY_RESET,
    DEVICE_CLASS_RESTART,
    DEVICE_CLASS_UPDATE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ENTITY_CATEGORY_NONE,
)

from .. import CONF_LD6001A_ID, LD6001AComponent, ld6001a_ns

DEPENDENCIES = ["ld6001a"]

StartButton = ld6001a_ns.class_("StartButton", button.Button)
StopButton = ld6001a_ns.class_("StopButton", button.Button)
HardResetButton = ld6001a_ns.class_("HardResetButton", button.Button)
SoftResetButton = ld6001a_ns.class_("SoftResetButton", button.Button)
FactoryResetButton = ld6001a_ns.class_("FactoryResetButton", button.Button)

CONF_START = "start"
CONF_STOP = "stop"
CONF_HARD_RESET = "hard_reset"
CONF_SOFT_RESET = "soft_reset"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD6001A_ID): cv.use_id(LD6001AComponent),
    cv.Optional(CONF_START): button.button_schema(
        StartButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_NONE,
    ),
    cv.Optional(CONF_STOP): button.button_schema(
        StopButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_NONE,
    ),
    cv.Optional(CONF_HARD_RESET): button.button_schema(
        HardResetButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_SOFT_RESET): button.button_schema(
        SoftResetButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_FACTORY_RESET): button.button_schema(
        FactoryResetButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}


async def to_code(config):
    ld6001a_component = await cg.get_variable(config[CONF_LD6001A_ID])
    if start_config := config.get(CONF_START):
        b = await button.new_button(start_config)
        await cg.register_parented(b, config[CONF_LD6001A_ID])
        cg.add(ld6001a_component.set_start_button(b))
    if stop_config := config.get(CONF_STOP):
        b = await button.new_button(stop_config)
        await cg.register_parented(b, config[CONF_LD6001A_ID])
        cg.add(ld6001a_component.set_stop_button(b))
    if hard_reset_config := config.get(CONF_HARD_RESET):
        b = await button.new_button(hard_reset_config)
        await cg.register_parented(b, config[CONF_LD6001A_ID])
        cg.add(ld6001a_component.set_hard_reset_button(b))
    if soft_reset_config := config.get(CONF_SOFT_RESET):
        b = await button.new_button(soft_reset_config)
        await cg.register_parented(b, config[CONF_LD6001A_ID])
        cg.add(ld6001a_component.set_soft_reset_button(b))
    if factory_reset_config := config.get(CONF_FACTORY_RESET):
        b = await button.new_button(factory_reset_config)
        await cg.register_parented(b, config[CONF_LD6001A_ID])
        cg.add(ld6001a_component.set_factory_reset_button(b))
