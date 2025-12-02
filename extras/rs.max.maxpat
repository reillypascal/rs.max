{
    "patcher": {
        "fileversion": 1,
        "appversion": {
            "major": 9,
            "minor": 1,
            "revision": 1,
            "architecture": "x64",
            "modernui": 1
        },
        "classnamespace": "box",
        "rect": [ 59.0, 119.0, 655.0, 470.0 ],
        "boxes": [
            {
                "box": {
                    "hidden": 1,
                    "id": "obj-3",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 70.0, 290.0, 92.0, 22.0 ],
                    "text": "help rs.file2sig~"
                }
            },
            {
                "box": {
                    "hidden": 1,
                    "id": "obj-20",
                    "linecount": 4,
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 389.0, 109.0, 155.0, 62.0 ],
                    "text": ";\rmax launchbrowser https://reillyspitzfaden.com/code#maxmsp"
                }
            },
            {
                "box": {
                    "fontsize": 14.0,
                    "id": "obj-15",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 389.0, 71.0, 166.0, 24.0 ],
                    "text": "reillyspitzfaden.com/code"
                }
            },
            {
                "box": {
                    "fontsize": 14.0,
                    "id": "obj-10",
                    "linecount": 5,
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 70.0, 290.0, 265.0, 85.0 ],
                    "text": "[rs.file2sig~]:  import any file as raw binary data and play this data back as a control signal for synthesizing PSK (phase-shift keying)/ASK/FSK/etc. telecommunications signals."
                }
            },
            {
                "box": {
                    "fontsize": 14.0,
                    "id": "obj-7",
                    "linecount": 3,
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 70.0, 115.0, 244.0, 53.0 ],
                    "text": "A collection of Max/MSP externals by Reilly Spitzfaden, including tools for databending and glitch sounds."
                }
            },
            {
                "box": {
                    "fontface": 1,
                    "fontsize": 24.0,
                    "id": "obj-5",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 70.0, 71.0, 93.0, 33.0 ],
                    "text": "rs.max"
                }
            },
            {
                "box": {
                    "hidden": 1,
                    "id": "obj-24",
                    "ignoreclick": 1,
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 70.0, 318.0, 52.0, 22.0 ],
                    "text": "pcontrol"
                }
            },
            {
                "box": {
                    "fontsize": 18.0,
                    "id": "obj-2",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 70.0, 255.0, 95.0, 29.0 ],
                    "text": "rs.file2sig~"
                }
            }
        ],
        "lines": [
            {
                "patchline": {
                    "destination": [ "obj-20", 0 ],
                    "hidden": 1,
                    "source": [ "obj-15", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-3", 0 ],
                    "hidden": 1,
                    "source": [ "obj-2", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-24", 0 ],
                    "hidden": 1,
                    "source": [ "obj-3", 0 ]
                }
            }
        ],
        "autosave": 0
    }
}