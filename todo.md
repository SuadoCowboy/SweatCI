- improve SweatCI to give more access for us by making the parser not a single function but actually a bunch of divided functions that can be called to make a manual parsing if we want to

- why was exec command saving the variables even though it was missing a '&'? Is it some weird memory issue?

- exec from .cfg also check path with the path of that cfg that is running