# scripple
A pebble app that scribes spontaneous thoughts.

![Screen0](/res/Screenshot0.png)&nbsp;&nbsp;
![Screen3](/res/Screenshot3.png)&nbsp;&nbsp;
![Screen2](/res/Screenshot2.png)

## Etymology
/ˈskripəl/

A hasty word play from scribe and pebble. In hindsight, should've just called it
_scribble_

## Details
This is a WatchApp for Pebble that will allow the user to add notes or thoughts
directly into Pebble via the Dictation service.

## Usage
- Click the `+` to start a dictation session to input text.
- Use the scroll buttons on the watch to move up and down.
- Click `Select` on an item to open it up in a more detailed view.
- Long press the `Select` button to delete an item.

## Purpose
The reason for this project is simply for me to code some more. And I also
happen to love my Pebble watch. I wanted to get the following from this project:

- Learn Pebble's Persistent Storage model
- Learn Pebble's Dictation Service
- Learn Pebble's UI and Menu Navigation
- Learn CloudPebble and how to publish a Pebble app
- Learn how to write idiomatic C
- Learn how to write efficient and performant C code

## Future Ideas
Currently I just want a simple create and delete of limited number of menu items.

This project could be expanded on its own to set reminders for a menu item. This
will involve learning about [Wakeups](https://developer.pebble.com/guides/events-and-services/wakeups/)
and perhaps even sync data with a companion phone app.
