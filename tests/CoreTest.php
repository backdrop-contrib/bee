<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console functionality.
 */

use PHPUnit\Framework\TestCase;

class CoreTest extends TestCase {

  /**
   * Make sure that `b` is installed correctly.
   */
  public function testBInstalledCorrectly() {
    // Capture the exit status of the command.
    exec('b', $output, $result);

    // A '0' exit status means success.
    $this->assertEquals(0, $result);
  }

  /**
   * Make sure that unknown commands display an error.
   */
  public function testUnknownCommandDisplaysError() {
    // Need to change this if we ever add a 'spoon' command ;-)
    $command = 'spoon';
    $output = shell_exec("b $command");

    $this->assertStringContainsString("There is no $command", $output);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function testRequiredArgumentsAreRequired() {
    // `config-get` has a required argument: 'config_name'.
    $output = shell_exec('b config-get');

    $this->assertStringContainsString('Argument config_name is required', $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function testCommandAliasesWork() {
    // `st` is an alias for `core-status`.
    $output_command = shell_exec('b core-status');
    $output_alias = shell_exec('b st');

    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--root` option works.
   */
  public function testRootOptionWorks() {
    // Test running `b` from inside Backdrop, outside Backdrop, and then with
    // the '--root' option.
    $output_backdrop = shell_exec('b st');
    $output_not_backdrop = shell_exec('cd ../ && b st');
    $output_root = shell_exec('cd ../ && b --root=www st');

    $this->assertStringContainsString('Backdrop CMS Installation detected', $output_backdrop);
    $this->assertStringContainsString('No Backdrop installation found', $output_not_backdrop);
    $this->assertEquals($output_backdrop, $output_root);
  }

  /**
   * Make sure that the `--y/--yes` options work.
   */
  public function testYesOptionWorks() {
    $module = 'book';
    $output_y = shell_exec("b en $module --y");
    $output_yes = shell_exec("b dis $module --yes");

    $this->assertStringContainsString("Module $module enabled", $output_y);
    $this->assertStringContainsString("Module $module disabled", $output_yes);
  }

}

