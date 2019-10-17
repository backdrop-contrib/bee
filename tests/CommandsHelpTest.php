<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Help commands.
 */

use PHPUnit\Framework\TestCase;

class CommandsHelpTest extends TestCase {

  /**
   * Check that the Help command works.
   */
  public function testHelp() {
    // Display help for the `b` command in general.
    $output_all = shell_exec('b help');
    $this->assertStringContainsString('Backdrop Console commands', $output_all);
    $this->assertStringContainsString('Provides a birds-eye view of the current Backdrop installation', $output_all);

    // Display help for the `help` command.
    $output_help = shell_exec('b help help');
    $this->assertStringContainsString('Display help', $output_help);
    $this->assertStringContainsString('Command to print help', $output_help);
  }

}

