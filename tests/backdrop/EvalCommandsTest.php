<?php
/**
 * @file
 * PHPUnit tests for Bee eval() commands.
 */

use PHPUnit\Framework\TestCase;

class EvalCommandsTest extends TestCase {

  /**
   * Make sure that the php-eval command works.
   */
  public function test_php_eval_command_works() {
    // Test a simple command.
    $output_all = shell_exec('bee php-eval \'echo "foo" . "bar";\'');
    $this->assertStringContainsString('foobar', $output_all);

    // Test that the terminal semicolon is optional.
    $output_all = shell_exec('bee php-eval \'echo "foo" . "bar"\'');
    $this->assertStringContainsString('foobar', $output_all);
  }

}
