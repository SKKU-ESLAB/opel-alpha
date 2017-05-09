package com.opel.cmfw.controller.bluetooth;

/**
 * Created by redcarrottt on 2017. 5. 9..
 */

public class FallbackException extends Exception {
    private static final long serialVersionUID = 1L;

    public FallbackException(Exception e) {
        super(e);
    }
}