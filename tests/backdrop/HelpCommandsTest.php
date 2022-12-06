<?php
/**
 * @file
 * PHPUnit tests for Bee Help commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's help command.
 */
class HelpCommandsTest extends TestCase {

  /**
   * Make sure that the help command works.
   */
  public function test_help_command_works() {
    // Display help for `bee` in general.
    $output_all = shell_exec('bee help');
    $this->assertStringContainsString('Usage: bee [global-options] <command> [options] [arguments]', (string) $output_all);
    $this->assertStringContainsString("Answer 'yes' to questions without prompting.", (string) $output_all);
    $this->assertStringContainsString('Clear a specific cache, or all Backdrop caches.', (string) $output_all);

    // Display help for the `help` command.
    $output_help = shell_exec('bee help help');
    $this->assertStringContainsString("Provide help and examples for 'bee' and its commands.", (string) $output_help);
    $this->assertStringContainsString('The command to display help for.', (string) $output_help);
  }

}
