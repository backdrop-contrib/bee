# Contributing
Thank you for your interest in contributing to Bee. Please take note and follow
the guidelines for contributing to this project.

## Issues
Creating issues and commenting on existing issues are the the first steps to
contributing.

### Bugs
If reporting a bug, please ensure you provide complete details of the bug
including:
- Steps to reproduce
- Expected results
- Actual results
- Relevant environment information could include, if it is applicable to the
bug:
  - Bee version (get by typing `bee version` or `bee status`)
  - Backdrop CMS version (get from `bee status`)
  - PHP version (get from `bee status`)
  - MySQL or MariaDB version
  - OS and version (Remember: Windows is not supported natively but Bee can be
  used within WSL2 and other virtual machines).
  - Version of executable (if applicable)

When you report a bug, please watch out for and respond promptly to any follow
up questions that I or other contributors may have in clarifying your bug
report.

### Enhancements
You are welcome to request new commands or improvements to existing commands.
Please provide clear details as to why you want the improvement or what the
command would be used for.

All contributors make contributions voluntarily so please be patient and don't
expect instant results.

If you see an existing enhancement request that you would like, then feel free
to add your support to that request.

### Questions
If you are unsure of some part of how bee operates and you have checked the
documentation in [the Wiki](https://github.com/backdrop-contrib/bee/wiki) feel
free to ask a question in the issue queue.

## Pull Requests
Pull requests must only relate to issues. You can indicate which issue it fixes
by starting the description of the pull request with:
`Fixes #000` where '000' is the issue number you are addressing. This will link
the pull request to the issue.

### Coding standards
All code is expected to adhere to both:
- [Backdrop CMS Coding and Documentation Standards](https://docs.backdropcms.org/documentation/coding-and-documentation-standards), specifically:
  - [PHP coding standards](https://docs.backdropcms.org/php-standards)
  - [Code documentation standards](https://docs.backdropcms.org/doc-standards)
- Existing conventions within Bee

Exceptions can be made where there are good reasons. For example, in the `eval`
command we have the following where we specifically ignore a check for a
discouraged PHP function on the next line:

```php
    // phpcs:ignore Squiz.PHP.Eval -- integral part of the command
    eval($arguments['code'] . ';');
```

### Tests
There are automated tests which test both functionality and coding standards,
though the coding standards test is not comprehensive. If tests fail, please
attempt to fix if you can. If you're not sure why tests have failed, ask.

If you are adding a new command or making changes to the way a command works, a
new functional test or changes to existing functional tests, respectively, may 
be required. It is ok to request help if you are unsure about this.

### Feedback and revisions
If you are given feedback in a review, please act on all of it; it is time
consuming and frustrating to have to ask for the same changes multiple times.
If you don't understand, please ask for clarification. If you don't agree,
please explain why you don't agree.

## Wiki
The Wiki is open to all to improve. New commands will need a change to the wiki
and changes to existing commands may require a change to the wiki.
Please follow existing patterns within the wiki and if you are unsure about
something, ask before making a change.

If you feel the Wiki is missing something but you don't know what the answer
is, feel free to ask a question in the issue queue.

Changes to the wiki are made by making an edit to the page with the `docs`
folder of the repository and creating a pull request linked to an issue to
merge it. This process was introduced to make it easier to build wiki updates
into the process of adding or changing commands.