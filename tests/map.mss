@all-fonts: "DejaVu Sans Book", "DejaVu Sans Bold", "DejaVu Sans Oblique";
@bold-fonts: "DejaVu Sans Bold";
@book-fonts: "DejaVu Sans Book";
@oblique-fonts: "DejaVu Sans Oblique";

Map {
    srs: "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    background-color: rgb(255, 255, 255);
}

#provinces {
    .provinces {
        [(NAME_EN='Ontario')] {
            polygon-fill: rgb(250, 190, 183);
        }
        [(NOM_FR='Québec')] {
            polygon-fill: rgb(217, 235, 203);
        }
    }
}

#quebec_hydrography {
    .drainage {
        [(HYC=8)] {
            polygon-fill: rgb(153, 204, 255);
        }
    }
}

#ontario_hydrography {
    .drainage {
        [(HYC=8)] {
            polygon-fill: rgb(153, 204, 255);
        }
    }
}

#provincial_borders {
    .provlines {
        {
            line-width: 1.0;
            line-dasharray: 8.0, 4.0, 2.0, 2.0, 2.0, 2.0;
        }
    }
}

#roads {
    .highway-border {
        [(CLASS=1)] {
            line-color: rgb(188, 149, 28);
            line-width: 7.0;
            line-join: round;
            line-cap: round;
        }
    }
    .highway-fill {
        [(CLASS=1)] {
            line-color: rgb(242, 191, 36);
            line-width: 5.0;
            line-join: round;
            line-cap: round;
        }
    }
    .road-border {
        [(CLASS=2)] {
            line-color: rgb(171, 158, 137);
            line-width: 4.0;
            line-join: round;
            line-cap: round;
        }
    }
    .road-fill {
        [(CLASS=2)] {
            line-color: rgb(255, 250, 115);
            line-width: 2.0;
            line-join: round;
            line-cap: round;
        }
    }
    .smallroads {
        [((CLASS=3) or (CLASS=4))] {
            line-color: rgb(171, 158, 137);
            line-width: 2.0;
            line-join: round;
            line-cap: round;
        }
    }
}

#populated_places {
    .popplaces {
        {
            text-name: "[GEONAME]";
            text-face-name: "DejaVu Sans Book";
            text-size: 10;
            text-halo-fill: rgb(255, 255, 200);
            text-halo-radius: 1.0;
            text-vertical-alignment: auto;
        }
    }
}