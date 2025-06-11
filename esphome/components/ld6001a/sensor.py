import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ANGLE,
    CONF_DISTANCE,
    DEVICE_CLASS_DISTANCE,
    UNIT_CENTIMETER,
    UNIT_DEGREES,
    UNIT_MILLIMETER,
)

from . import CONF_LD6001A_ID, LD6001AComponent, MAX_ZONES

DEPENDENCIES = ["ld6001a"]

CONF_PITCH_ANGLE = "pitch_angle"
CONF_HORIZONTAL_ANGLE = "horizontal_angle"
CONF_MOVING_TARGET_COUNT = "moving_target_count"
CONF_STILL_TARGET_COUNT = "still_target_count"
CONF_TARGET_COUNT = "target_count"
CONF_X = "x"
CONF_Y = "y"
CONF_Z = "z"

ICON_ACCOUNT_GROUP = "mdi:account-group"
ICON_ACCOUNT_SWITCH = "mdi:account-switch"
ICON_ALPHA_X_BOX_OUTLINE = "mdi:alpha-x-box-outline"
ICON_ALPHA_Y_BOX_OUTLINE = "mdi:alpha-y-box-outline"
ICON_ALPHA_Z_BOX_OUTLINE = "mdi:alpha-z-box-outline"
ICON_FORMAT_TEXT_ROTATION_ANGLE_UP = "mdi:format-text-rotation-angle-up"
ICON_ANGLE_ACUTE = "mdi:angle-acute"
ICON_HUMAN_GREETING_PROXIMITY = "mdi:human-greeting-proximity"
ICON_MAP_MARKER_ACCOUNT = "mdi:map-marker-account"
ICON_MAP_MARKER_DISTANCE = "mdi:map-marker-distance"
ICON_RELATION_ZERO_OR_ONE_TO_ZERO_OR_ONE = "mdi:relation-zero-or-one-to-zero-or-one"
ICON_SPEEDOMETER_SLOW = "mdi:speedometer-slow"

MAX_TARGETS = 10

UNIT_MILLIMETER_PER_SECOND = "mm/s"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001A_ID): cv.use_id(LD6001AComponent),
        cv.Optional(CONF_TARGET_COUNT): sensor.sensor_schema(
            icon=ICON_ACCOUNT_GROUP,
        ),
        cv.Optional(CONF_STILL_TARGET_COUNT): sensor.sensor_schema(
            icon=ICON_HUMAN_GREETING_PROXIMITY,
        ),
        cv.Optional(CONF_MOVING_TARGET_COUNT): sensor.sensor_schema(
            icon=ICON_ACCOUNT_SWITCH,
        ),
    }
)

CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
    {
        cv.Optional(f"target_{n + 1}"): cv.Schema(
            {
                cv.Optional(CONF_X): sensor.sensor_schema(
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    icon=ICON_ALPHA_X_BOX_OUTLINE,
                ),
                cv.Optional(CONF_Y): sensor.sensor_schema(
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    icon=ICON_ALPHA_Y_BOX_OUTLINE,
                ),
                cv.Optional(CONF_Z): sensor.sensor_schema(
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    icon=ICON_ALPHA_Z_BOX_OUTLINE,
                ),
                cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    icon=ICON_MAP_MARKER_DISTANCE,
                ),
            }
        )
        for n in range(MAX_TARGETS)
    },
    {
        cv.Optional(f"zone_{n + 1}"): cv.Schema(
            {
                cv.Optional(CONF_TARGET_COUNT): sensor.sensor_schema(
                    icon=ICON_MAP_MARKER_ACCOUNT,
                ),
            }
        )
        for n in range(MAX_ZONES)
    },
)


async def to_code(config):
    ld6001a_component = await cg.get_variable(config[CONF_LD6001A_ID])

    if target_count_config := config.get(CONF_TARGET_COUNT):
        sens = await sensor.new_sensor(target_count_config)
        cg.add(ld6001a_component.set_target_count_sensor(sens))

    for n in range(MAX_TARGETS):
        if target_conf := config.get(f"target_{n + 1}"):
            if x_config := target_conf.get(CONF_X):
                sens = await sensor.new_sensor(x_config)
                cg.add(ld6001a_component.set_move_x_sensor(n, sens))
            if y_config := target_conf.get(CONF_Y):
                sens = await sensor.new_sensor(y_config)
                cg.add(ld6001a_component.set_move_y_sensor(n, sens))
            if z_config := target_conf.get(CONF_Z):
                sens = await sensor.new_sensor(z_config)
                cg.add(ld6001a_component.set_move_z_sensor(n, sens))
            if distance_config := target_conf.get(CONF_DISTANCE):
                sens = await sensor.new_sensor(distance_config)
                cg.add(ld6001a_component.set_move_distance_sensor(n, sens))

    for n in range(MAX_ZONES):
        if zone_config := config.get(f"zone_{n + 1}"):
            if target_count_config := zone_config.get(CONF_TARGET_COUNT):
                sens = await sensor.new_sensor(target_count_config)
                cg.add(ld6001a_component.set_zone_target_count_sensor(n, sens))
