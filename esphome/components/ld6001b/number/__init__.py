import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_ILLUMINANCE,
    ENTITY_CATEGORY_CONFIG,
    ICON_LIGHTBULB,
    ICON_MOTION_SENSOR,
    UNIT_SECOND,
    UNIT_CENTIMETER,
)

from .. import CONF_LD6001B_ID, LD6001BComponent, ld6001b_ns, MAX_ZONES

GroundRadiusNumber = ld6001b_ns.class_("GroundRadiusNumber", number.Number)
CoordinateType = ld6001b_ns.enum("CoordinateType")
LongDistanceSensitivityNumber = ld6001b_ns.class_("LongDistanceSensitivityNumber", number.Number)
InstallationHeightNumber = ld6001b_ns.class_("InstallationHeightNumber", number.Number)
HeartBeatNumber = ld6001b_ns.class_("HeartBeatNumber", number.Number)
MonitorIntervalNumber = ld6001b_ns.class_("MonitorIntervalNumber", number.Number)
ScanIntervalNumber = ld6001b_ns.class_("ScanIntervalNumber", number.Number)
CoordinateNumber = ld6001b_ns.class_("CoordinateNumber", number.Number)


CONF_GROUND_RADIUS = "ground_radius"
CONF_INSTALLATION_HEIGHT = "installation_height"
CONF_LONG_DISTANCE_SENSITIVITY = "long_distance_sensitivity"
CONF_SENSITIVITY = "sensitivity"
CONF_HEARTBEAT = "heart_beat"
CONF_MONITOR_INTERVAL = "monitor_interval"
CONF_SCAN_INTERVAL = "scan_interval"

CONF_X1 = "x1"
CONF_X2 = "x2"
CONF_Y1 = "y1"
CONF_Y2 = "y2"
ICON_ARROW_BOTTOM_RIGHT = "mdi:arrow-bottom-right"
ICON_ARROW_BOTTOM_RIGHT_BOLD_BOX_OUTLINE = "mdi:arrow-bottom-right-bold-box-outline"
ICON_ARROW_TOP_LEFT = "mdi:arrow-top-left"
ICON_ARROW_TOP_LEFT_BOLD_BOX_OUTLINE = "mdi:arrow-top-left-bold-box-outline"

ZoneCoordinateNumber = ld6001b_ns.class_("ZoneCoordinateNumber", number.Number)

RANGE_CONFS = [
    ["x_min", CoordinateType.X_MIN, -500, -20],
    ["x_max", CoordinateType.X_MAX, 20, 500],
    ["y_min", CoordinateType.Y_MIN, -500, -20],
    ["y_max", CoordinateType.Y_MAX, 20, 500],
]

TIMEOUT_GROUP = "timeout"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001B_ID): cv.use_id(LD6001BComponent),
        cv.Optional(CONF_GROUND_RADIUS): number.number_schema(
            GroundRadiusNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            unit_of_measurement=UNIT_CENTIMETER,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
        cv.Optional(CONF_LONG_DISTANCE_SENSITIVITY): number.number_schema(
            LongDistanceSensitivityNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
        cv.Optional(CONF_HEARTBEAT): number.number_schema(
            HeartBeatNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            unit_of_measurement=UNIT_SECOND,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
        cv.Optional(CONF_INSTALLATION_HEIGHT): number.number_schema(
            InstallationHeightNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            unit_of_measurement=UNIT_CENTIMETER,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
        cv.Optional(CONF_MONITOR_INTERVAL): number.number_schema(
            HeartBeatNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            unit_of_measurement=UNIT_SECOND,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
        cv.Optional(CONF_SCAN_INTERVAL): number.number_schema(
            HeartBeatNumber,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            unit_of_measurement=UNIT_SECOND,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_LIGHTBULB,
        ),
    }
)

CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
    {
        cv.Optional(f"zone_{n + 1}"): cv.Schema(
            {
                cv.Required(CONF_X1): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_TOP_LEFT_BOLD_BOX_OUTLINE,
                ),
                cv.Required(CONF_Y1): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_TOP_LEFT,
                ),
                cv.Required(CONF_X2): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_BOTTOM_RIGHT_BOLD_BOX_OUTLINE,
                ),
                cv.Required(CONF_Y2): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_BOTTOM_RIGHT,
                ),
            }
        )
        for n in range(MAX_ZONES)
    }
)

CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
    {
        cv.Optional(data[0]): number.number_schema(
            CoordinateNumber,
            device_class=DEVICE_CLASS_DISTANCE,
            unit_of_measurement=UNIT_CENTIMETER,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_MOTION_SENSOR,
        )

        for data in RANGE_CONFS
    }
)

async def to_code(config):
    ld6001b_component = await cg.get_variable(config[CONF_LD6001B_ID])

    if range_config := config.get(CONF_GROUND_RADIUS):
        n = await number.new_number(
            range_config, min_value=100, max_value=500, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_ground_radius_number(n))

    if sensitivity_config := config.get(CONF_SENSITIVITY):
        n = await number.new_number(
            sensitivity_config, min_value=1, max_value=19, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_sensitivity_number(n))

    if long_distance_sensitivity_config := config.get(CONF_LONG_DISTANCE_SENSITIVITY):
        n = await number.new_number(
            long_distance_sensitivity_config, min_value=1, max_value=9, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_long_distance_sensitivity_number(n))

    if installation_height_config := config.get(CONF_INSTALLATION_HEIGHT):
        n = await number.new_number(
            installation_height_config, min_value=50, max_value=500, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_installation_height_number(n))

    if heartbeat_config := config.get(CONF_HEARTBEAT):
        n = await number.new_number(
            heartbeat_config, min_value=10, max_value=999, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_heartbeat_number(n))

    if monitor_interval_config := config.get(CONF_MONITOR_INTERVAL):
        n = await number.new_number(
            monitor_interval_config, min_value=1, max_value=99, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_monitor_interval_number(n))

    if scan_interval_config := config.get(CONF_SCAN_INTERVAL):
        n = await number.new_number(
            scan_interval_config, min_value=100, max_value=10000, step=1
        )
        await cg.register_parented(n, config[CONF_LD6001B_ID])
        cg.add(ld6001b_component.set_scan_interval_number(n))

    for [coordinate_name, coordinate_type, min, max] in RANGE_CONFS:
        if range_config := config.get(coordinate_name):
            n = await number.new_number(
                range_config, coordinate_type, min_value=min, max_value=max, step=1
            )

            await cg.register_parented(n, config[CONF_LD6001B_ID])

            setter = f"set_{coordinate_name}_number"

            cg.add(getattr(ld6001b_component, setter)(n))

    for zone_num in range(MAX_ZONES):
        if zone_conf := config.get(f"zone_{zone_num + 1}"):
            zone_x1_config = zone_conf.get(CONF_X1)


            x1 = cg.new_Pvariable(zone_x1_config[CONF_ID], zone_num)
            x1_n = await number.register_number(
                x1, zone_x1_config, min_value=-500, max_value=500, step=1
            )

            await cg.register_parented(x1, config[CONF_LD6001B_ID])

            zone_y1_config = zone_conf.get(CONF_Y1)
            y1 = cg.new_Pvariable(zone_y1_config[CONF_ID], zone_num)
            await number.register_number(
                y1, zone_y1_config, min_value=-500, max_value=500, step=1
            )
            await cg.register_parented(y1, config[CONF_LD6001B_ID])

            zone_x2_config = zone_conf.get(CONF_X2)
            x2 = cg.new_Pvariable(zone_x2_config[CONF_ID], zone_num)
            await number.register_number(
                x2, zone_x2_config, min_value=-500, max_value=500, step=1
            )
            await cg.register_parented(x2, config[CONF_LD6001B_ID])

            zone_y2_config = zone_conf.get(CONF_Y2)
            y2 = cg.new_Pvariable(zone_y2_config[CONF_ID], zone_num)
            await number.register_number(
                y2, zone_y2_config, min_value=-500, max_value=500, step=1
            )
            await cg.register_parented(y2, config[CONF_LD6001B_ID])

            cg.add(ld6001b_component.set_zone_numbers(zone_num, x1, y1, x2, y2))

