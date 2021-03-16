<?php
/**
 * @file
 * PHPUnit tests for Bee Help commands.
 */

use PHPUnit\Framework\TestCase;

class HelpCommandsTest extends TestCase {

  /**
   * Make sure that the help command works.
   */
  public function test_help_command_works() {
    // Display help for `bee` in general.
    $output_all = shell_exec('bee help');
    $this->assertStringContainsString('Usage: bee [global-options] <command> [options] [arguments]', $output_all);
    $this->assertStringContainsString("Answer 'yes' to questions without prompting.", $output_all);
    $this->assertStringContainsString('Clear a specific cache, or all Backdrop caches.', $output_all);

    // Display help for the `help` command.
    $output_help = shell_exec('bee help help');
    $this->assertStringContainsString("Provide help and examples for 'bee' and its commands.", $output_help);
    $this->assertStringContainsString('The command to display help for.', $output_help);
  }

}
